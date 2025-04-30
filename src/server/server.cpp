#include "server.hpp"

// std
#include <algorithm>
#include <cstring> // memset
#include <fcntl.h>
#include <format>

// wut
#include <arpa/inet.h> // ネット関連の処理に必要
#include <nn/ac.h> // IPアドレスの取得に必要
#include <sys/socket.h>
#include <unistd.h> // close関数に必要
#include <whb/log.h>

// stardust
#include "command.hpp"
#include "../exception/callback.hpp"
#include "../transfer/packet.hpp"
#include "../utils/bytes.hpp"

using namespace std;

namespace TCPServer
{
    const uint16_t TCP_PORT = 10000;  // クライアント側はこのポートで接続する

    jthread tcpServer;  // TCPサーバー用のスレッド

    int32_t serverSocket = -1; // サーバー側のソケット
    int32_t clientSocket = -1; // クライアント側のソケット
    uint32_t hostIpAddress = 0; // WiiUのIPアドレスを格納する変数

    // 初期化
    void Init()
    {
        if(Start() != -1)
        {
            tcpServer = jthread(Thread);
        }
    }

    // 初期化解除
    void Deinit()
    {
        End();
    }

    // スレッド
    void Thread(stop_token token)
    {
        while(!token.stop_requested())
        {
            Update();
        }
    }

    // ネットを初期化しIPアドレスを取得
    void InitSocketFunctionPointers()
    {
        WHBLogPrintf("Initializing network functions");
        nn::ac::Initialize();
        nn::ac::Connect();
        int result = nn::ac::GetAssignedAddress(&hostIpAddress);
        if(result != 1)
        {
            WHBLogPrintf("Failed to get IP address:%d", result);
            return;
        }
        WHBLogPrintf("Succeeded to get IP address");
    }

    // 開始
    int Start()
    {
        WHBLogPrintf("Starting TCP server");
        InitSocketFunctionPointers();

        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(serverSocket < 0)
        {
            WHBLogPrintf("Failed to create socket\n");
            return -1;
        }
        WHBLogPrintf("Socket created");

        // ソケットを再利用できるように設定
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            WHBLogPrintf("setsockopt failed");
            close(serverSocket);
            return 1;
        }

        // IPアドレスやポート番号を設定
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(TCP_PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            close(serverSocket);
            serverSocket = -1;
            WHBLogPrintf("Failed to bind socket\n");
            return -1;
        }
        WHBLogPrintf("Socket bound");

        if(listen(serverSocket, 1) < 0)
        {
            WHBLogPrintf("Failed to listen on socket\n");
            return -1;
        }
        WHBLogPrintf("Listening on socket");

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &hostIpAddress, ipStr, INET_ADDRSTRLEN);
        WHBLogPrintf("Server started on IP: %s, port: %d\n", ipStr, TCP_PORT);
        return 0;
    }

    // 更新
    void Update()
    {
        AcceptClient();
        ReceiveFromClient();
        UpdateOther();
    }

    // 終了
    void End()
    {
        WHBLogPrintf("Ending TCP server");

        if(clientSocket != -1)
        {
            fcntl(clientSocket, F_SETFL, O_NONBLOCK);
            shutdown(clientSocket, 0);
            close(clientSocket);
            clientSocket = -1; // クライアントソケットをリセット
            WHBLogPrintf("Client reset");
        }

        if(serverSocket != -1)
        {
            fcntl(serverSocket, F_SETFL, O_NONBLOCK);
            shutdown(serverSocket, 0);
            close(serverSocket);
            serverSocket = -1;  // サーバーソケットをリセット
            WHBLogPrintf("Server reset");
        }

        if (tcpServer.joinable())
        {
            tcpServer.request_stop();
            tcpServer.join();
            WHBLogPrintf("Server thread ended");
        }

        WHBLogPrintf("Server ended");
    }

    // クライアントの接続待機
    void AcceptClient()
    {
        if(serverSocket != -1 && clientSocket == -1)
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

    void DisconnectClient()
    {
        if(clientSocket != -1)
        {
            close(clientSocket);
            clientSocket = -1; // クライアントソケットをリセット
        }
    }

    void ReceiveFromClient()
    {
        if(serverSocket != -1 && clientSocket != -1)
        {
            int32_t result; // 関数の返り値を格納する変数

            // データを受け取るためのバッファ
            Packet packet;
            result = packet.ReceivePacket(clientSocket); // データを受け取る
            if(result == 0)
            {
                DisconnectClient();
            }

            if(packet.Type == Packet::DATA_TYPE_DATA)
            {
                ProcessCommand(clientSocket, packet.Data); // コマンドを処理
            }
        }
    }

    void UpdateOther()
    {
        if(Exception::isNotification)
        {
            string message = format("DSI Exception: {:08X}", Exception::contextForNotification.srr0);
            NotificationModule_AddInfoNotification(message.c_str());
            Exception::isNotification = false; // 通知が完了したらフラグをリセット
        }
    }
}