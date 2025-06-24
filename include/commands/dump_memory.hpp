#pragma once

#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

#include <notifications/notifications.h>

#include "packet.hpp"

namespace Command
{
    // メモリダンプコマンド
    void DumpMemory(Packet& packet);
}

namespace Memory
{
    void Dump(uint32_t start, uint32_t size, Command::ValueType valueType, bool compress);
    void Dump_SaveMetadata(uint32_t start, uint32_t size, Command::ValueType valueType, bool compressed);
    void Dump_LoadMetadata(uint32_t& outStart, uint32_t& outSize, Command::ValueType& outValueType, bool& outCompressed);
}