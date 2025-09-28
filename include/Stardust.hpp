#pragma once

#include <StardustLib/MessageServer.hpp>

namespace Stardust
{
    class Stardust
    {
    private:
        const uint16_t port = 50000;
        std::unique_ptr<StardustLib::MessageServer> server = nullptr;
    
        static void OnServerStart(uint32_t ipAddress);
        static void OnClientConnect(uint32_t ipAddress, uint32_t id);

    public:
        void initialize();
        void shutdown();
    };
    
    extern std::unique_ptr<Stardust> gStardust;
}