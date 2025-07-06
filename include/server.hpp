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

// stardust
#include "socket_context.hpp"

class ThreadWorkerBase
{
public:
    std::jthread thread;
    virtual ~ThreadWorkerBase() = default;

    void Start(std::function<void(std::stop_token)> func)
    {
        thread = std::jthread(func);
    }

    virtual void Stop()
    {
        if (thread.joinable())
        {
            thread.request_stop();
            thread.join();
        }
    }

protected:
    virtual void Main(std::stop_token token) = 0;
};

class Server
{
public:
    static std::unique_ptr<Server> Create();
    bool Init();
    void Shutdown();
    ~Server();
    
private:
    static constexpr const int32_t SOCKET_NULL = -1; // 空のソケット
    static constexpr const uint32_t IP_ADDRESS_NULL = 0; // 空のIPアドレス
    static constexpr const uint32_t SERVER_BACKLOG = 128; // ソケットの最大同時待機数
    uint16_t port = 50000;
    uint32_t ipAddress;
    int serverSocket;
    std::vector<std::shared_ptr<SocketContext>> clientContexts;
    std::mutex clientContextsMutex;

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
    void SetSocketNoDelay(); // TCP_NODELAYオプションを設定
    void SetServerInfo(sockaddr_in& info); // IPアドレスやポート番号を設定
    void BindSocket(sockaddr_in& info); // ソケットにアドレス情報をバインド
    void ListenSocket();
    void NotificationIPAddress();

    // 終了関数
    void CloseClientSocket();
    void CloseServerSocket();

    // クライアントへのデータ送信
    bool SendToClient(std::shared_ptr<SocketContext> client, const std::vector<uint8_t>& data);
    void Broadcast(const std::vector<uint8_t>& data);

    class AcceptThread : public ThreadWorkerBase
    {
    public:
        Server* server = nullptr;
        void Main(std::stop_token token) override;
    };

    AcceptThread acceptThread;

    class ReceiveThread : public ThreadWorkerBase
    {
    public:
        Server* server = nullptr;
        void Main(std::stop_token token) override;
    };

    ReceiveThread receiveThread;
    std::queue<std::pair<std::shared_ptr<SocketContext>, std::vector<uint8_t>>> receiveQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondVar;

    class ProcessThread : public ThreadWorkerBase
    {
    public:
        Server* server = nullptr;
        void Main(std::stop_token token) override;
        void Stop() override; // Override to notify condition variable
    };

    ProcessThread processThread;
};

extern std::unique_ptr<Server> gServer;