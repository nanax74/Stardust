#pragma once

// stardust
#include "packet.hpp"

namespace Command
{
    void CheckSearchFile(Packet& packet);
}

namespace Memory
{
    bool CheckSearchFile();
}