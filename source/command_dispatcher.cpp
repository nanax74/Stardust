#include "command_dispatcher.hpp"
#include <format>
#include <cstring> // memcpy
#include <thread>
#include <notifications/notifications.h>

#include "commands/common.hpp"
#include "commands/write_memory.hpp"
#include "commands/specific_search.hpp"

std::unordered_map<uint32_t, CommandDispatcher::HandlerFunc> CommandDispatcher::handlers;

void CommandDispatcher::RegisterHandlers()
{
    // メモリ書き込み
    handlers[Command::ID_WriteMemory] = Command::WriteMemory;

    // メモリ検索
    handlers[Command::ID_SpecificSearch] = Command::SpecificSearch;
}

void CommandDispatcher::Dispatch(Packet& packet)
{
    if (packet.Size() < sizeof(uint32_t))
    {
        throw std::runtime_error("Packet too small to contain command ID");
    }

    uint32_t cmd = packet.Read<uint32_t>();
    auto it = handlers.find(cmd);
    if (it == handlers.end())
    {
        throw std::runtime_error("Unknown command ID");
    }

    auto& handler = it->second;
    handler(packet);
}
