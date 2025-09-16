#pragma once

#include <StardustLib/MessageServer.hpp>

namespace Stardust
{
    class Stardust
    {
    private:
        const uint16_t port = 50000;
        std::unique_ptr<StardustLib::MessageServer> server = nullptr;
    
    public:
        void initialize();
        void shutdown();
    };
    
    extern std::unique_ptr<Stardust> gStardust;
}