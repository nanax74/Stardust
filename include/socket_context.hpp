#pragma once

#include <mutex>

#include <atomic>
#include <arpa/inet.h>

struct SocketContext
{
    int socket;
    std::mutex sendMutex;
    std::mutex recvMutex;
    std::atomic<bool> connected = true;
    sockaddr_in address;
};