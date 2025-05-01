#include "server.hpp"

// std
#include <algorithm>
#include <cstring> // memset
#include <fcntl.h>
#include <format>

// wut
#include <nn/ac.h> // IPアドレスの取得に必要
#include <sys/socket.h>
#include <unistd.h> // close関数に必要
#include <whb/log.h>

// stardust
#include "command.hpp"
#include "../transfer/packet.hpp"
#include "../utils/bytes.hpp"

TCPServer::TCPServer(uint16_t port)
{
    this->port = port;
    serverSocket = SOCKET_NULL;
    clientSocket = SOCKET_NULL;
    ipAddress = IP_ADDRESS_NULL;
    Init();
}

TCPServer::~TCPServer()
{
    Deinit();
}

void TCPServer::Init()
{
    // スレッドをスタート
    tcpServer = std::jthread
    (
        [this](std::stop_token stoken)
        {
            Main(stoken);
        }
    );
}

void TCPServer::Deinit()
{
    End();
}

// ネットを初期化しIPアドレスを取得
void TCPServer::InitSocketFunctionPointers()
{
    WHBLogPrintf("Initializing network functions");
    nn::ac::Initialize();
    nn::ac::Connect();
    nn::Result result = nn::ac::GetAssignedAddress(&ipAddress);
    if(result.IsFailure())
    {
        throw std::runtime_error("Failed to get IP address");
    }
    WHBLogPrintf("Succeeded to get IP address");
}

// ソケットの作成
void TCPServer::CreateSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == SOCKET_NULL)
    {
        throw std::runtime_error("Failed to create socket");
    }
    WHBLogPrintf("Succeeded to create socket");
}

// ソケットを再利用できるように設定
void TCPServer::SetSocketReusable()
{
    int option = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        throw std::runtime_error("Failed to set option: SO_REUSEADDR");
    }
}

// IPアドレスやポート番号を設定
void TCPServer::SetServerInfo(sockaddr_in& info)
{
    memset(&info, 0, sizeof(info));
    info.sin_family = AF_INET;
    info.sin_port = htons(port);
    info.sin_addr.s_addr = htonl(INADDR_ANY);
}

// ソケットにアドレス情報をバインド
void TCPServer::BindSocket(sockaddr_in& info)
{
    if(bind(serverSocket, (struct sockaddr*)&info, sizeof(info)) < 0)
    {
        throw std::runtime_error("Failed to bind socket");
    }
    WHBLogPrintf("Socket bound");
}

void TCPServer::ListenSocket()
{
    if(listen(serverSocket, 1) < 0)
    {
        throw std::runtime_error("Failed to listen on socket");
    }
    WHBLogPrintf("Listening on socket");
}

void TCPServer::PrintIPLog()
{
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ipAddress, ipStr, INET_ADDRSTRLEN);
    WHBLogPrintf("Server started on IP: %s, port: %d\n", ipStr, port);
}

void TCPServer::CloseClientSocket()
{
    if(clientSocket != SOCKET_NULL)
    {
        fcntl(clientSocket, F_SETFL, O_NONBLOCK);
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = SOCKET_NULL; // クライアントソケットをリセット
        WHBLogPrintf("Client reset");
    }
}

void TCPServer::CloseServerSocket()
{
    if(serverSocket != SOCKET_NULL)
    {
        fcntl(serverSocket, F_SETFL, O_NONBLOCK);
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = SOCKET_NULL;  // サーバーソケットをリセット
        WHBLogPrintf("Server reset");
    }
}

void TCPServer::StopServerThread()
{
    if (tcpServer.joinable())
    {
        tcpServer.request_stop();
        tcpServer.join();
        WHBLogPrintf("Server thread ended");
    }
}

// クライアントの接続待機
void TCPServer::AcceptClient()
{
    if(serverSocket != SOCKET_NULL && clientSocket == SOCKET_NULL)
    {
        WHBLogPrintf("Waiting to accept client");
        clientSocket = accept(serverSocket, NULL, NULL);
        if(clientSocket < 0)
        {
            WHBLogPrintf("Failed to accept client");
            return;
        }
        WHBLogPrintf("Client connected:%X Server:%X", clientSocket, serverSocket);
    }
}

void TCPServer::ReceiveFromClient()
{
    if(serverSocket != SOCKET_NULL && clientSocket != SOCKET_NULL)
    {
        int32_t result; // 関数の返り値を格納する変数
        // データを受け取るためのバッファ
        Packet packet;
        result = packet.ReceivePacket(clientSocket); // データを受け取る
        if(result == 0)
        {
            CloseClientSocket();
        }
        if(packet.Type == Packet::DATA_TYPE_DATA)
        {
            Command::ProcessCommand(clientSocket, packet.Data); // コマンドを処理
        }
    }
}

// スレッド
void TCPServer::Main(std::stop_token token)
{
    // 初期化
    Start();
    while(!token.stop_requested())
    {
        // 更新
        Update();
    }
}

// サーバーのセットアップ
bool TCPServer::Start()
{
    try
    {
        WHBLogPrintf("Starting TCP server");
        InitSocketFunctionPointers();
        CreateSocket();
        SetSocketReusable();
        sockaddr_in serverInfo;
        SetServerInfo(serverInfo);
        BindSocket(serverInfo);
        ListenSocket();
        PrintIPLog();
        WHBLogPrintf("Started TCP server");
        return true;
    }
    catch(const std::exception& e)
    {
        WHBLogPrintf(e.what());
        if (serverSocket != SOCKET_NULL)
        {
            close(serverSocket);
            serverSocket = SOCKET_NULL;
        }
        return false;
    }
}

void TCPServer::Update()
{
    AcceptClient();
    ReceiveFromClient();
}

void TCPServer::End()
{
    WHBLogPrintf("Ending TCP server");
    CloseClientSocket();
    CloseServerSocket();
    StopServerThread();
    nn::ac::Finalize();
    WHBLogPrintf("Server ended");
}