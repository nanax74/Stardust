#include "server.hpp"
#include <fcntl.h> // 非ブロッキングソケットの設定に必要

namespace TCPServer
{
    int serverSocket;
    int clientSocket;
    unsigned int hostIpAddress = 0;

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
        WHBLogPrintf("Update called");
        AcceptClient();
        ReceiveData();
    }

    void Stop()
    {
        WHBLogPrintf("Stopping TCP server");
        close(clientSocket);
        close(serverSocket);
        WHBLogPrintf("Server stopped");
    }

    void AcceptClient()
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

    void ReceiveData()
    {
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            WHBLogPrintf("Received: %s\n", buffer);
        }
        else if (bytesReceived < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
        {
            WHBLogPrintf("Failed to receive data\n");
        }
    }

    void SendData(const char* data)
    {
        send(clientSocket, data, strlen(data), 0);
    }
}