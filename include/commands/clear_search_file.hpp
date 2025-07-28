#pragma once

// stardust
#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class ClearSearchFile : public Base
    {
        public:
            ClearSearchFile() : Base("ClearSearchFile") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    void ClearSearchFile();
}