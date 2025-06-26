// std
#include <chrono>
#include <fcntl.h>
#include <memory> // unique_ptr
#include <poll.h>
#include <string>
#include <thread>
#include <unistd.h> // close関数に必要

// wut
#include <netinet/tcp.h> // TCP_NODELAY
#include <nn/ac.h> // WiiUのIPアドレスを取得
#include <notifications/notifications.h>
#include <whb/log.h>

// stardust
#include "command_dispatcher.hpp"
#include "server.hpp"

std::unique_ptr<Server> gServer = nullptr;

std::unique_ptr<Server> Server::Create()
{
    std::unique_ptr<Server> server(new Server());
    if (!server->Init())
    {
        return nullptr;
    }
    return server;
}

Server::Server() : ipAddress(IP_ADDRESS_NULL), serverSocket(SOCKET_NULL)
{
    clientContexts.clear();
}

bool Server::Init()
{
    try
    {
        InitSocketFunctionPointers();
        CreateSocket();
        SetSocketReusable();
        SetSocketNonBlocking();

        sockaddr_in serverInfo;
        SetServerInfo(serverInfo);
        BindSocket(serverInfo);
        ListenSocket();
        NotificationIPAddress();

        CommandDispatcher::RegisterHandlers();

        // スレッド起動（完全に初期化された後）
        acceptThread.server = this;
        acceptThread.thread = std::jthread([this](std::stop_token t)
        {
            acceptThread.Main(t);
        });

        receiveThread.server = this;
        receiveThread.thread = std::jthread([this](std::stop_token t)
        {
            receiveThread.Main(t);
        });

        processThread.server = this;
        processThread.thread = std::jthread([this](std::stop_token t)
        {
            processThread.Main(t);
        });

        return true;
    }
    catch (const std::exception& e)
    {
        WHBLogPrintf("Server::Init failed: %s", e.what());
        if (serverSocket != SOCKET_NULL)
        {
            close(serverSocket);
            serverSocket = SOCKET_NULL;
        }
        return false;
    }
}

void Server::Shutdown()
{
    acceptThread.Stop();
    receiveThread.Stop();
    processThread.Stop();
    CloseClientSocket();
    CloseServerSocket();
    nn::ac::Finalize();
}

Server::~Server()
{
    Shutdown();
}

// ネットを初期化しIPアドレスを取得
void Server::InitSocketFunctionPointers()
{
    if (nn::ac::Initialize().IsFailure())
    {
        throw std::runtime_error("Failed to connect network");
    }

    if (nn::ac::Connect().IsFailure())
    {
        throw std::runtime_error("Failed to connect network");
    }

    if(nn::ac::GetAssignedAddress(&ipAddress).IsFailure())
    {
        throw std::runtime_error("Failed to get IP address");
    }
}

// ソケットの作成
void Server::CreateSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == SOCKET_NULL)
    {
        throw std::runtime_error("Failed to create socket");
    }
}

// ソケットを再利用できるように設定
void Server::SetSocketReusable()
{
    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        throw std::runtime_error("Failed to set option: SO_REUSEADDR");
    }
}

// 非ブロッキングに設定
void Server::SetSocketNonBlocking()
{
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("Failed to get socket flags");
    }

    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

// TCP_NODELAYオプションを設定
void Server::SetSocketNoDelay()
{
    int option = 1;
    if (setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, &option, sizeof(option)) < 0)
    {
        throw std::runtime_error("Failed to set option: TCP_NODELAY");
    }
}

// IPアドレスやポート番号を設定
void Server::SetServerInfo(sockaddr_in& info)
{
    memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    info.sin_addr.s_addr = htonl(INADDR_ANY);
}

// ソケットにアドレス情報をバインド
void Server::BindSocket(sockaddr_in& info)
{
    if(bind(serverSocket, (struct sockaddr*)&info, sizeof(info)) < 0)
    {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Server::ListenSocket()
{
    if (listen(serverSocket, SERVER_BACKLOG) < 0)
    {
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Server::NotificationIPAddress()
{
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddress, ipStr, INET_ADDRSTRLEN);
    std::string ipAddress = std::format("Server started on IP: {}, port: {}", ipStr, port);
    NotificationModule_AddInfoNotification(ipAddress.c_str());
}

void Server::CloseClientSocket()
{
    std::lock_guard<std::mutex> lock(clientContextsMutex);

    for (auto& ctx : clientContexts)
    {
        if (ctx && ctx->connected.exchange(false))
        {
            shutdown(ctx->socket, SHUT_RDWR);
            close(ctx->socket);
        }
    }

    clientContexts.clear(); // 全クライアントを消去
}

void Server::CloseServerSocket()
{
    if(serverSocket != SOCKET_NULL)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = SOCKET_NULL;  // サーバーソケットをリセット
    }
}

bool Server::SendToClient(std::shared_ptr<SocketContext> client, const std::vector<uint8_t>& data)
{
    if (!client || !client->connected) return false;

    std::lock_guard<std::mutex> lock(client->sendMutex);
    ssize_t sent = send(client->socket, data.data(), data.size(), 0);

    return sent == (ssize_t)data.size();
}

void Server::Broadcast(const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(clientContextsMutex);
    for (auto& ctx : clientContexts)
    {
        SendToClient(ctx, data);
    }
}

// クライアントの接続待機
void Server::AcceptThread::Main(std::stop_token token)
{
    NotificationModule_AddInfoNotification("Waiting for client to connect");

    while (!token.stop_requested())
    {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(server->serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSock < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MILLISECONDS));
                continue;
            }
            std::string msg = std::format(" {}", errno);
            WHBLogPrintf("Accept failed: %s", strerror(errno));
            continue;
        }

        auto clientContext = std::make_shared<SocketContext>();
        clientContext->socket = clientSock;
        clientContext->connected = true;
        clientContext->address = clientAddr;

        {
            std::lock_guard<std::mutex> lock(server->clientContextsMutex);
            server->clientContexts.push_back(clientContext);
        }

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
        std::string msg = std::format("Client connected: {} Socket: {}", ipStr, clientSock);
        NotificationModule_AddInfoNotification(msg.c_str());
    }
}

void Server::AcceptThread::Stop()
{
    if (thread.joinable())
    {
        thread.request_stop();
        thread.join();
    }
}

// スレッド
void Server::ReceiveThread::Main(std::stop_token token)
{
    constexpr size_t BUFFER_SIZE = 0x1000;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (!token.stop_requested())
    {
        std::vector<struct pollfd> pollfds;
        std::vector<std::shared_ptr<SocketContext>> contexts;

        {
            std::lock_guard<std::mutex> lock(server->clientContextsMutex);
            for (const auto& ctx : server->clientContexts)
            {
                if (!ctx->connected) continue;

                struct pollfd pfd;
                pfd.fd = ctx->socket;
                pfd.events = POLLIN;
                pfd.revents = 0;
                pollfds.push_back(pfd);
                contexts.push_back(ctx);
            }
        }

        if (pollfds.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MILLISECONDS));
            continue;
        }

        int ret = poll(pollfds.data(), pollfds.size(), 100);
        if (ret < 0)
        {
            WHBLogPrintf("poll failed");
            break;
        }
        else if (ret == 0)
        {
            continue;
        }

        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN)
            {
                auto ctx = contexts[i];
                std::lock_guard<std::mutex> recvLock(ctx->recvMutex);

                ssize_t received = recv(ctx->socket, buffer.data(), buffer.size(), 0);

                if (received == 0)
                {
                    std::string msg = std::format("Client disconnected: Socket {}", ctx->socket);
                    NotificationModule_AddInfoNotification(msg.c_str());
                    shutdown(ctx->socket, SHUT_RDWR);
                    close(ctx->socket);
                    ctx->connected = false;
                }
                else if (received < 0)
                {
                    std::string msg = std::format("Recv error on socket {}: errno {}", ctx->socket, errno);
                    WHBLogPrintf(msg.c_str());
                    shutdown(ctx->socket, SHUT_RDWR);
                    close(ctx->socket);
                    ctx->connected = false;
                }
                else
                {
                    ctx->recvBuffer.insert(ctx->recvBuffer.end(), buffer.begin(), buffer.begin() + received);
                
                    auto packets = Packet::ReceivePackets(ctx->recvBuffer, ctx);
                    {
                        std::lock_guard<std::mutex> queueLock(server->queueMutex);
                        for (auto& pkt : packets)
                        {
                            server->receiveQueue.push(std::make_pair(ctx, pkt.GetData())); // or push(pkt)
                        }
                    }
                    if (!packets.empty())
                        server->queueCondVar.notify_one();
                }
            }
        }

        // 無効なクライアントを削除
        {
            std::lock_guard<std::mutex> lock(server->clientContextsMutex);
            auto& vec = server->clientContexts;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                [](const std::shared_ptr<SocketContext>& ctx)
                {
                    return !ctx->connected;
                }), vec.end());
        }
    }
}

void Server::ReceiveThread::Stop()
{
    if (thread.joinable())
    {
        thread.request_stop();
        thread.join();
    }
}

void Server::ProcessThread::Main(std::stop_token token)
{
    while (!token.stop_requested())
    {
        std::unique_lock<std::mutex> lock(server->queueMutex);

        // 条件変数で通知を待つ（キューが空でなければすぐ進む）
        server->queueCondVar.wait(lock, [&]()
        {
            return !server->receiveQueue.empty() || token.stop_requested();
        });

        if (token.stop_requested())
        {
            break;
        }

        // キューからデータを取り出す
        auto [ctx, data] = std::move(server->receiveQueue.front());
        server->receiveQueue.pop();
        lock.unlock();  // 早めにロックを解放

        // 実際のデータ処理
        try
        {
            Packet packet(data, ctx);
            CommandDispatcher::Dispatch(packet);
        }
        catch (const std::exception& e)
        {
            std::string msg = std::format("Processing error: {}", e.what());
            WHBLogPrintf(msg.c_str());
        }

        // 処理負荷軽減のため適宜スリープも検討（必要なら）
        std::this_thread::sleep_for(std::chrono::milliseconds(THREAD_SLEEP_TIME_MILLISECONDS));
    }
}

void Server::ProcessThread::Stop()
{
    if (thread.joinable())
    {
        thread.request_stop();

        {
            std::lock_guard<std::mutex> lock(server->queueMutex);
            server->queueCondVar.notify_one();
        }

        thread.join();
    }
}