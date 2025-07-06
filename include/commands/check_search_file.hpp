#pragma once

// stardust
#include "packet.hpp"

namespace Command
{
    // サーチをクリア
    void CheckSearchFile(Packet& packet);
}

namespace Memory
{
    // サーチをクリア
    bool CheckSearchFile();
}