#pragma once

#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class ReadMultipleMemory : public Base
    {
        public:
            ReadMultipleMemory() : Base("ReadMultipleMemory") {}
            void Process(Packet& packet) override;
    };
}