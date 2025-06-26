#pragma once

// stardust
#include "packet.hpp"

namespace Command
{
    // サーチをクリア
    void ClearSearchFile(Packet& packet);
}

namespace Memory
{
    // サーチをクリア
    void ClearSearchFile();
}