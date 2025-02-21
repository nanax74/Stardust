#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <whb/log.h>
#include <nn/ac.h>
#include <unistd.h>
#include <cstring>
#include "../main.hpp"

namespace TCPServer
{
    void Start();
    void Update();
    void Stop();
    void AcceptClient();
    void ReceiveData();
    void SendData(const char* data);
}