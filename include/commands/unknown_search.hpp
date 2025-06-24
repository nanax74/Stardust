#pragma once

#include "commands/common.hpp"

// std
#include <filesystem>
#include <fstream>

// wut
#include <coreinit/memorymap.h>

// wums
#include <notifications/notifications.h>

// stardust
#include "packet.hpp"

namespace Command
{
    // 未知の値を検索するコマンド
    void UnknownSearch(Packet& packet);
}

namespace Memory
{
    template <typename T>
    void UnknownSearch(Command::Comparison comparison, uint32_t alignment, T value)
    {

    }
}
