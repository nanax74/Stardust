#pragma once

#include <Stardust/TCPServer.hpp>

class Stardust
{
private:
    const uint16_t port = 50000;
    std::unique_ptr<StardustLib::TCPServer> server = nullptr;

    void recvCallback(const StardustLib::TCPServer::Packet& data);
    void disconnectCallback(uint64_t clientId);
    void serverIPAddressCallback(uint32_t ipAddress);
    void clientIPAddressCallback(uint32_t ipAddress, uint64_t id);

public:
    void initialize();
    void shutdown();
};

extern std::unique_ptr<Stardust> gStardust;