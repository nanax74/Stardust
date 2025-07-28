#pragma once

// stardust
#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class CheckSearchFile : public Base
    {
        public:
            CheckSearchFile() : Base("CheckSearchFile") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    bool CheckSearchFile();
}