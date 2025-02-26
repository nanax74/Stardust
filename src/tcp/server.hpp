#pragma once

#include "../main.hpp"
#include <coreinit/memdefaultheap.h> // メモリ操作に必要
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <whb/log.h>
#include <nn/ac.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <fcntl.h> // 非ブロッキングソケットの設定に必要
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include "command.hpp"
#include "data.hpp"
#include "../utils/bytes.hpp"

namespace TCPServer
{
    void Init();
    void Deinit();
    void Thread(std::stop_token token);
    void Start();
    void Update();
    void End();
    void AcceptClient();
    void DisconnectClient();
    void ReceiveFromClient();
}