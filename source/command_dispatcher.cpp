#include "command_dispatcher.hpp"
#include <format>
#include <cstring> // memcpy
#include <thread>
#include <notifications/notifications.h>

#include "commands/write_memory.hpp"

std::unordered_map<uint32_t, CommandDispatcher::HandlerFunc> CommandDispatcher::handlers;

void CommandDispatcher::RegisterHandlers()
{
    // メモリ書き込み
    handlers[1] = Command::WriteMemory;

    // メモリ検索
    handlers[2] = [](Packet& packet)
    {
        uint32_t start = packet.Read<uint32_t>();
        uint32_t end   = packet.Read<uint32_t>();
        uint32_t val   = packet.Read<uint32_t>();

        for (uint32_t addr = start; addr < end; addr += 4)
        {
            if (*((uint32_t*)addr) == val)
            {
                NotificationModule_AddInfoNotification(
                    std::format("Found at 0x{:08X}", addr).c_str()
                );
                return;
            }
        }
        NotificationModule_AddInfoNotification("Value not found");
    };
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
