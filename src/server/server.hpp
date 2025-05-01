#pragma once

// std
#include <thread>

// wut
#include <arpa/inet.h> // ネット関連の処理に必要

class TCPServer
{
public:
    TCPServer(uint16_t port);
    ~TCPServer();

    void Init();
    void Deinit();

    TCPServer(const TCPServer&) = delete;
    TCPServer& operator=(const TCPServer&) = delete;

private:
    std::jthread tcpServer;  // TCPサーバー用のスレッド

private:
    const int32_t SOCKET_NULL = -1; // 空のソケット
    const uint32_t IP_ADDRESS_NULL = 0; // 空のIPアドレス

private:
    uint16_t port; // ポート番号
    uint32_t ipAddress; // WiiUのIPアドレスを格納する変数
    int32_t serverSocket; // サーバー側のソケット
    int32_t clientSocket; // クライアント側のソケット

private:
    // ネットを初期化しIPアドレスを取得
    void InitSocketFunctionPointers();

    // ソケットの作成
    void CreateSocket();

    // ソケットを再利用できるように設定
    void SetSocketReusable();

    // IPアドレスやポート番号を設定
    void SetServerInfo(sockaddr_in& info);

    // ソケットにアドレス情報をバインド
    void BindSocket(sockaddr_in& info);

    void ListenSocket();

    void PrintIPLog();

private:
    void CloseClientSocket();

    void CloseServerSocket();

    void StopServerThread();

private:
    // クライアントの接続待機
    void AcceptClient();

private:
    void ReceiveFromClient();

private:
    // スレッド
    void Main(std::stop_token token);

    // サーバーのセットアップ
    bool Start();

    void Update();

    void End();
};