#pragma once

#include <coreinit/memdefaultheap.h> // メモリ操作に必要
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <whb/log.h>
#include <nn/ac.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h> // 非ブロッキングソケットの設定に必要
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "../main.hpp"
#include "command.hpp"

namespace TCPServer
{
    void Init();
    void Thread(std::stop_token token);
    void Start();
    void Update();
    void Stop();
    void AcceptClient();
    void ReceiveData();
    void SendData(unsigned char* buffer, int length);
}