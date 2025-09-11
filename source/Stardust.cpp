// wut
#include <arpa/inet.h> // WiiUのIPアドレスを取得

// Stardust-Lib
#include <Stardust/TCPServer.hpp>

// Stardust
#include "Stardust.hpp"
#include "notification.hpp"
#include "logger.hpp"

std::unique_ptr<Stardust> gStardust = nullptr;

void Stardust::recvCallback(const StardustLib::TCPServer::Packet& data)
{
    Notification::Info("Data received. Client ID: {}, Size: {}", data.clientId, data.data.size());
}

void Stardust::disconnectCallback(uint64_t clientId)
{
    Notification::Info("Client disconnected with ID: {}", clientId);
}

void Stardust::serverIPAddressCallback(uint32_t ipAddress)
{
    struct in_addr addr;
    addr.s_addr = ipAddress;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN);
    
    Notification::Info("Server started on IP: {}, Port: {}", ipStr, port);
}

void Stardust::clientIPAddressCallback(uint32_t ipAddress, uint64_t id)
{
    struct in_addr addr;
    addr.s_addr = ipAddress;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, ipStr, INET_ADDRSTRLEN);

    Notification::Info("Client connected on IP: {}, Port: {}, ID: {}", ipStr, port, id);
}

void Stardust::initialize()
{
    if(!server)
    {
        server = std::make_unique<StardustLib::TCPServer>(port);
        server->setRecvCallback([this](const StardustLib::TCPServer::Packet& data){ recvCallback(data); });
        server->setDisconnectCallback([this](uint64_t clientId){ disconnectCallback(clientId); });
        server->setServerIPAddressCallback([this](int ipAddress){ serverIPAddressCallback(ipAddress); });
        server->setClientIPAddressCallback([this](int ipAddress, uint64_t id){ clientIPAddressCallback(ipAddress, id); });
    }
    server->start();
}

void Stardust::shutdown()
{
    if(server) server->stop();
}