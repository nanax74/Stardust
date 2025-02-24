#include "server.hpp"

using namespace std;

namespace TCPServer
{
    #define TCP_PORT 10000  // クライアント側はこのポートで接続する

    jthread tcpServer;  // TCPサーバー用のスレッド

    int serverSocket = -1; // サーバー側のソケット
    int clientSocket = -1; // クライアント側のソケット
    unsigned int hostIpAddress = 0; // WiiUのIPアドレスを格納する変数

    // 初期化
    void Init()
    {
        Start();
        tcpServer = jthread(Thread);
    }

    // 初期化解除
    void Deinit()
    {
        End();
    }

    // スレッド
    void Thread(stop_token token)
    {
        while (!token.stop_requested())
        {
            Update();
            this_thread::sleep_for(chrono::milliseconds(1000 / 60));
        }
    }

    // ネットを初期化しIPアドレスを取得
    void InitSocketFunctionPointers()
    {
        WHBLogPrintf("Initializing network functions");
        nn::ac::Initialize();
        nn::ac::Connect();
        nn::ac::GetAssignedAddress(&hostIpAddress);
        WHBLogPrintf("Network functions initialized");
    }

    // 開始
    void Start()
    {
        WHBLogPrintf("Starting TCP server");
        InitSocketFunctionPointers();

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket < 0)
        {
            WHBLogPrintf("Failed to create socket\n");
            return;
        }
        WHBLogPrintf("Socket created");

        // IPアドレスやポート番号を設定
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(TCP_PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            WHBLogPrintf("Failed to bind socket\n");
            return;
        }
        WHBLogPrintf("Socket bound");

        if (listen(serverSocket, 1) < 0)
        {
            WHBLogPrintf("Failed to listen on socket\n");
            return;
        }
        WHBLogPrintf("Listening on socket");

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &hostIpAddress, ipStr, INET_ADDRSTRLEN);
        WHBLogPrintf("Server started on IP: %s, port: %d\n", ipStr, TCP_PORT);
    }

    // 更新
    void Update()
    {
        AcceptClient();
        ReceiveFromClient();
    }

    // 終了
    void End()
    {
        WHBLogPrintf("Ending TCP server");

        tcpServer.request_stop();

        if (clientSocket != -1)
        {
            close(clientSocket);
            clientSocket = -1; // クライアントソケットをリセット
        }

        if (serverSocket != -1)
        {
            close(serverSocket);
            serverSocket = -1;  // サーバーソケットをリセット
        }

        WHBLogPrintf("Server ended");
    }

    // クライアントの接続待機
    void AcceptClient()
    {
        if (clientSocket == -1)
        {
            WHBLogPrintf("Waiting to accept client");
            clientSocket = accept(serverSocket, NULL, NULL);
            WHBLogPrintf("Client connected");
        }
    }

    void ReceiveFromClient()
    {
        bytes data = ReceiveData(clientSocket);
        ProcessCommand(clientSocket, data);
    }

    // データ送信関数
    void SendData(int socket, const bytes& data)
    {
        // データの長さを送信
        int dataLength = data.size();
        int lengthSent = send(socket, reinterpret_cast<const char*>(&dataLength), sizeof(dataLength), 0);
        if (lengthSent == -1)
        {
            WHBLogPrintf("Failed to send data length.");
            return;
        }

        // データ本体を送信
        int totalSent = 0;
        while (totalSent < dataLength)
        {
            int sent = send(socket, reinterpret_cast<const char*>(data.data()) + totalSent, min(4096, dataLength - totalSent), 0);
            if (sent == -1)
            {
                WHBLogPrintf("Failed to send data.");
                return;
            }
            totalSent += sent;
        }
    }

    // データ受信関数
    bytes ReceiveData(int socket)
    {
        // まずデータの長さを受信
        int dataLength = 0;
        int lengthReceived = recv(socket, reinterpret_cast<char*>(&dataLength), sizeof(dataLength), 0);
        if (lengthReceived == -1)
        {
            WHBLogPrintf("Failed to receive data length.");
            return {};
        }

        // 受信するデータを格納するためのバッファを準備
        bytes data;
        int totalReceived = 0;
        while (totalReceived < dataLength)
        {
            int received = recv(socket, reinterpret_cast<char*>(data.data()) + totalReceived, min(4096, dataLength - totalReceived), 0);
            if (received == -1)
            {
                WHBLogPrintf("Failed to receive data.");
                return {};
            }
            totalReceived += received;
        }

        return data;
    }
}