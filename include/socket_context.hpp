#pragma once

#include <mutex>

#include <atomic>
#include <arpa/inet.h>

struct SocketContext
{
    int socket;
    std::atomic<bool> connected = true;
    sockaddr_in address;

    std::mutex sendMutex;
    std::mutex recvMutex;

    std::vector<uint8_t> recvBuffer;
};