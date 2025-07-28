#pragma once

// stardust
#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class Respond : public Base
    {
        public:
            Respond() : Base("Respond") {}
            void Process(Packet& packet) override;
    };
}