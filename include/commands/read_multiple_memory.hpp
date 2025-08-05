#pragma once

#include "commands/common.hpp"
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

    bool ReadMemoryAndWritePacketValue(Packet& packet, ValueType valueType, uint32_t address);
}