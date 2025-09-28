// Stardust-Lib
#include <StardustLib/MessageServer.hpp>
#include <StardustLib/IPAddress.hpp>

// Stardust
#include "Stardust.hpp"
#include "notification.hpp"
#include "logger.hpp"

// messages
#include "Messages/Ping.hpp"
#include "Messages/Pong.hpp"

// wut
#include <arpa/inet.h>
#include <netinet/in.h>

// std
#include <string>

namespace Stardust
{
    std::unique_ptr<Stardust> gStardust = nullptr;
    
    constexpr uint16_t MSG_TYPE_PING = 1;
    constexpr uint16_t MSG_TYPE_PONG = 2;

    void Stardust::OnServerStart(uint32_t ipAddress)
    {
        std::string ipStr = StardustLib::to_string(ipAddress);
        Notification::Info("Server started. IPAddress: {}", ipStr);
    }
    
    void Stardust::OnClientConnect(uint32_t ipAddress, uint32_t id)
    {
        std::string ipStr = StardustLib::to_string(ipAddress);
        Notification::Info("Client connected. IPAddress: {}, ID: {}", ipStr, id);
    }

    void Stardust::initialize()
    {
        if(!server)
        {
            server = std::make_unique<StardustLib::MessageServer>(port, OnServerStart, OnClientConnect);
            server->registerType<Ping>(MSG_TYPE_PING);
            server->registerType<Pong>(MSG_TYPE_PONG);
        }
        server->start();
    }
    
    void Stardust::shutdown()
    {
        if(server) server->stop();
    }
}