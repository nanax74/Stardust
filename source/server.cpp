// std
#include <chrono>
#include <fcntl.h>
#include <memory> // unique_ptr
#include <poll.h>
#include <string>
#include <thread>

// wut
#include <netinet/tcp.h> // TCP_NODELAY
#include <nn/ac.h> // WiiUのIPアドレスを取得
#include <notifications/notifications.h>
#include <unistd.h> // close関数に必要

// stardust
#include "command_dispatcher.hpp"
#include "server.hpp"

std::unique_ptr<Server> gServer = nullptr;

std::unique_ptr<Server> Server::Create()
{
    std::unique_ptr<Server> server(new Server());
    if (!server->initialized)
    {
        return nullptr;
    }
    return server;
}

Server::Server()
{
    // 変数の初期化
    serverSocket = SOCKET_NULL;
    clientSockets.clear();
    ipAddress = IP_ADDRESS_NULL;

    try
    {
        // ソケットの初期化
        InitSocketFunctionPointers();
        CreateSocket();
        SetSocketReusable();
        SetSocketNonBlocking();
        sockaddr_in serverInfo;
        SetServerInfo(serverInfo);
        BindSocket(serverInfo);
        ListenSocket();
        NotificationIPAddress();

        CommandDispatcher::RegisterHandlers(); // コマンドハンドラの登録

        initialized = true;
    }
    catch(const std::exception& e)
    {
        // 例外処理
        NotificationModule_AddErrorNotification(e.what());
        if (serverSocket != SOCKET_NULL)
        {
            close(serverSocket);
            serverSocket = SOCKET_NULL;
        }
        initialized = false;
        return;
    }

    // 接続ループを開始
    acceptThread.thread = std::jthread
    (
        [this](std::stop_token token)
        {
            acceptThread.Main(token, this);
        }
    );

    // 受信ループを開始
    receiveThread.thread = std::jthread
    (
        [this](std::stop_token token)
        {
            receiveThread.Main(token, this);
        }
    );

    // 処理ループを開始
    processThread.thread = std::jthread
    (
        [this](std::stop_token token)
        {
            processThread.Main(token, this);
        }
    );
}

Server::~Server()
{
    CloseClientSocket();
    CloseServerSocket();
    acceptThread.Stop();
    receiveThread.Stop();
    processThread.Stop();
    nn::ac::Finalize();
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
    std::lock_guard<std::mutex> lock(clientSocketsMutex); // マルチスレッド安全対策
    for(int socket : clientSockets)
    {
        if(socket != SOCKET_NULL)
        {
            shutdown(socket, SHUT_RDWR);
            close(socket);
        }
    }
    clientSockets.clear(); // 完全に消去
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

// クライアントの接続待機
void Server::AcceptThread::Main(std::stop_token token, Server* server)
{
    NotificationModule_AddInfoNotification("Waiting for client to connect");

    while(!token.stop_requested())
    {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSock = accept(server->serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if(clientSock < 0)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            std::string msg = std::format("Accept failed: {}", errno);
            NotificationModule_AddErrorNotification(msg.c_str());
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(server->clientSocketsMutex);
            server->clientSockets.push_back(clientSock);
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
void Server::ReceiveThread::Main(std::stop_token token, Server* server)
{
    constexpr size_t BUFFER_SIZE = 0x1000;
    std::vector<uint8_t> buffer(BUFFER_SIZE);

    while (!token.stop_requested())
    {
        std::vector<struct pollfd> pollfds;
        {
            std::lock_guard<std::mutex> lock(server->clientSocketsMutex);
            for (int sock : server->clientSockets)
            {
                struct pollfd pfd;
                pfd.fd = sock;
                pfd.events = POLLIN; // 読み込み可能なイベント
                pfd.revents = 0;
                pollfds.push_back(pfd);
            }
        }

        if (pollfds.empty())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int ret = poll(pollfds.data(), pollfds.size(), 100); // タイムアウト100ms
        if (ret < 0)
        {
            NotificationModule_AddErrorNotification("poll failed");
            break;
        }
        else if (ret == 0)
        {
            continue; // タイムアウト：何もせず続行
        }

        std::lock_guard<std::mutex> lock(server->clientSocketsMutex);
        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN)
            {
                int clientSock = pollfds[i].fd;

                ssize_t received = recv(clientSock, buffer.data(), buffer.size(), 0);

                if (received == 0)
                {
                    std::string msg = std::format("Client disconnected: Socket {}", clientSock);
                    NotificationModule_AddInfoNotification(msg.c_str());
                    shutdown(clientSock, SHUT_RDWR);
                    close(clientSock);
                    server->clientSockets.erase(std::remove(
                        server->clientSockets.begin(),
                        server->clientSockets.end(),
                        clientSock
                    ), server->clientSockets.end());
                }
                else if (received < 0)
                {
                    std::string msg = std::format("Recv error on socket {}: errno {}", clientSock, errno);
                    NotificationModule_AddErrorNotification(msg.c_str());
                    shutdown(clientSock, SHUT_RDWR);
                    close(clientSock);
                    server->clientSockets.erase(std::remove(
                        server->clientSockets.begin(),
                        server->clientSockets.end(),
                        clientSock
                    ), server->clientSockets.end());
                }
                else
                {
                    std::vector<uint8_t> packet(buffer.begin(), buffer.begin() + received);
                    {
                        std::lock_guard<std::mutex> queueLock(server->queueMutex);
                        server->receiveQueue.push(std::make_pair(clientSock, std::move(packet)));
                    }
                    server->queueCondVar.notify_one();
                }
            }
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

void Server::ProcessThread::Main(std::stop_token token, Server* server)
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
        auto [clientSock, data] = std::move(server->receiveQueue.front());
        server->receiveQueue.pop();
        lock.unlock();  // 早めにロックを解放

        // 実際のデータ処理
        try
        {
            Packet packet(data, clientSock);
            CommandDispatcher::Dispatch(packet);
        }
        catch (const std::exception& e)
        {
            std::string msg = std::format("Processing error: {}", e.what());
            NotificationModule_AddErrorNotification(msg.c_str());
        }

        // 処理負荷軽減のため適宜スリープも検討（必要なら）
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Server::ProcessThread::Stop()
{
    if (thread.joinable())
    {
        thread.request_stop();
        thread.join();
    }
}