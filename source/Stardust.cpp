// Stardust-Lib
#include <StardustLib/MessageServer.hpp>

// Stardust
#include "Stardust.hpp"
#include "notification.hpp"
#include "logger.hpp"

// messages
#include "Messages/Ping.hpp"
#include "Messages/Pong.hpp"

namespace Stardust
{
    std::unique_ptr<Stardust> gStardust = nullptr;
    
    constexpr uint16_t MSG_TYPE_PING = 1;
    constexpr uint16_t MSG_TYPE_PONG = 2;
    
    void Stardust::initialize()
    {
        if(!server)
        {
            server = std::make_unique<StardustLib::MessageServer>(port);
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