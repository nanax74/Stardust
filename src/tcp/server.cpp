#include "server.hpp"

using namespace std;

namespace TCPServer
{
    jthread tcpServer;

    int serverSocket;
    int clientSocket = -1; // 初期値を-1に設定
    unsigned int hostIpAddress = 0;

    void Init()
    {
        Start();
        tcpServer = jthread(Thread);
    }

    void Thread(stop_token token)
    {
        while (!token.stop_requested())
        {
            Update();
            this_thread::sleep_for(chrono::milliseconds(1000 / 60));
        }
    }
    
    void InitSocketFunctionPointers()
    {
        WHBLogPrintf("Initializing network functions");
        nn::ac::Initialize();
        nn::ac::Connect();
        nn::ac::GetAssignedAddress(&hostIpAddress);
        WHBLogPrintf("Network functions initialized");
    }

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

        // 非ブロッキングモードに設定
        int flags = fcntl(serverSocket, F_GETFL, 0);
        fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);

        // IPアドレスやポート番号を設定
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(7000);
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
        WHBLogPrintf("Server started on IP: %s, port: 7000\n", ipStr);
    }

    void Update()
    {
        AcceptClient();
        ReceiveData();
    }

    void Stop()
    {
        WHBLogPrintf("Stopping TCP server");

        tcpServer.request_stop();

        if (clientSocket != -1)
        {
            close(clientSocket);
            clientSocket = -1; // クライアントソケットをリセット
        }
        close(serverSocket);

        WHBLogPrintf("Server stopped");
    }

    void AcceptClient()
    {
        if (clientSocket == -1)
        {
            WHBLogPrintf("Waiting to accept client");
            clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    WHBLogPrintf("Failed to accept client\n");
                }
                return;
            }
            WHBLogPrintf("Client connected");

            // クライアントソケットも非ブロッキングモードに設定
            int flags = fcntl(clientSocket, F_GETFL, 0);
            fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
        }
    }

    void ReceiveData()
    {
        if (clientSocket != -1)
        {
            unsigned char buffer[1024]; // 1KBまで受け取れる

            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            
            if (bytesReceived > 0)
            {
                ProcessCommand(buffer, bytesReceived);
            }
            else if (bytesReceived == 0)
            {
                WHBLogPrintf("Client disconnected");
                close(clientSocket);
                clientSocket = -1;
            }
            else if (bytesReceived < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
            {
                WHBLogPrintf("Failed to receive data\n");
            }
        }
    }

    void SendData(unsigned char* buffer, int length)
    {
        // クライアントが接続されていたらデータを送信
        if (clientSocket != -1)
        {
            send(clientSocket, buffer, length, 0);
        }
    }
}