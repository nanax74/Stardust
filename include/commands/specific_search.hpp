#pragma once

#include "commands/common.hpp"
#include "packet.hpp"

namespace Command
{
    void SpecificSearch(Packet& packet);
}

namespace Memory
{
    template <typename T>
    void SpecificSearch(uint32_t startAddress, uint32_t endAddress, Command::Comparison comparison, T searchValue, std::shared_ptr<SocketContext> ctx);
}