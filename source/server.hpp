#pragma once

// std
#include <cstdint>
#include <memory> // unique_ptr
#include <mutex> // スレッドへの同時アクセスを制限
#include <queue>
#include <thread>
#include <vector>

// wut
#include <arpa/inet.h>

class Server
{
public:
    static std::unique_ptr<Server> Create();
    ~Server();
    
private:
    static constexpr const int32_t SOCKET_NULL = -1; // 空のソケット
    static constexpr const uint32_t IP_ADDRESS_NULL = 0; // 空のIPアドレス
    static constexpr const uint32_t SERVER_BACKLOG = 128; // ソケットの最大同時待機数
    uint16_t port = 50000;
    uint32_t ipAddress;
    int serverSocket;
    std::mutex clientSocketsMutex;  // clientSocketsへのアクセスを保護するミューテックス
    std::vector<int> clientSockets;

    Server();

    // コピー禁止
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // 移動禁止
    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;

    bool initialized = false;

    // 開始関数
    void InitSocketFunctionPointers(); // ネットを初期化しIPアドレスを取得
    void CreateSocket(); // ソケットの作成
    void SetSocketReusable(); // ソケットを再利用できるように設定
    void SetSocketNonBlocking(); // 非ブロッキングに設定
    void SetServerInfo(sockaddr_in& info); // IPアドレスやポート番号を設定
    void BindSocket(sockaddr_in& info); // ソケットにアドレス情報をバインド
    void ListenSocket();
    void NotificationIPAddress();

    // 終了関数
    void CloseClientSocket();
    void CloseServerSocket();

    class ReceiveThread
    {
    public:
        std::jthread thread;
        void Main(std::stop_token token, Server* server);
        void Stop();
    };

    ReceiveThread receiveThread;
    std::queue<std::vector<uint8_t>> receiveQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;

    class AcceptThread
    {
    public:
        std::jthread thread;
        void Main(std::stop_token token, Server* server);
        void Stop();
    };

    AcceptThread acceptThread;

    class ProcessThread
    {
    public:
        std::jthread thread;
        void Main(std::stop_token token, Server* server);
        void Stop();
    };

    ProcessThread processThread;
};

extern std::unique_ptr<Server> gServer;