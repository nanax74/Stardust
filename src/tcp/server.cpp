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
        ReceiveData();
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

    // データ受信
    void ReceiveData()
    {
        if (clientSocket != -1)
        {
            // 読み取り可能なバイト数を取得
            int bytesAvailable;
            if (ioctl(clientSocket, FIONREAD, &bytesAvailable) < 0)
            {
                WHBLogPrintf("Failed to get available bytes\n");
                return;
            }

            if (bytesAvailable > 0)
            {
                vector<unsigned char> buffer(bytesAvailable);

                int bytesReceived = recv(clientSocket, buffer.data(), bytesAvailable, 0);
                WHBLogPrintf("Bytes Received:%d", bytesReceived);
                ProcessCommand(buffer.data(), bytesReceived);
            }
            else
            {
                // recvを呼び出してクライアントの切断を確認
                char temp;
                int bytesReceived = recv(clientSocket, &temp, 1, MSG_PEEK);

                if (bytesReceived == 0)
                {
                    // クライアントが切断された
                    WHBLogPrintf("Client disconnected");
                    close(clientSocket);
                    clientSocket = -1;
                }
                else if (bytesReceived < 0)
                {
                    // recvがエラーを返し、かつ EWOULDBLOCK / EAGAIN 以外なら切断とみなす
                    WHBLogPrintf("errno:%d, bytesReceived:%d", errno, bytesReceived);
                }
            }
        }
    }

    // データ送信
    void SendData(unsigned char* buffer, int length)
    {
        // クライアントが接続されていたらデータを送信
        if (clientSocket != -1)
        {
            send(clientSocket, buffer, length, 0);
        }
    }
}