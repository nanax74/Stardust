#include "command_dispatcher.hpp"

#include <format>
#include <cstring> // memcpy
#include <thread>

#include <notifications/notifications.h>

#include <whb/log.h>

#include "commands/common.hpp"
#include "commands/write_memory.hpp"
#include "commands/read_memory.hpp"
#include "commands/read_multiple_memory.hpp"
#include "commands/dump_for_search.hpp"
#include "commands/specific_search.hpp"
#include "commands/unknown_search.hpp"
#include "commands/clear_search_file.hpp"
#include "commands/check_search_file.hpp"
#include "commands/respond.hpp"

std::unordered_map<Command::ID, CommandDispatcher::HandlerFunc> CommandDispatcher::handlers;

void CommandDispatcher::RegisterHandlers()
{
    // メモリ書き込み
    handlers[Command::ID::WriteMemory] = Command::WriteMemory;

    // メモリ読み込み
    handlers[Command::ID::ReadMemory] = Command::ReadMemory;

    // 複数メモリ読み込み
    handlers[Command::ID::ReadMultipleMemory] = Command::ReadMultipleMemory;

    // メモリダンプ
    handlers[Command::ID::DumpForSearch] = Command::DumpForSearch;

    // 既知の値検索
    handlers[Command::ID::SpecificSearch] = Command::SpecificSearch;

    // 未知の値検索
    handlers[Command::ID::UnknownSearch] = Command::UnknownSearch;

    // サーチ関係のファイルを消去
    handlers[Command::ID::ClearSearchFile] = Command::ClearSearchFile;

    // サーチ関係のファイルの有無を確認
    handlers[Command::ID::CheckSearchFile] = Command::CheckSearchFile;

    // 応答を返すだけ
    handlers[Command::ID::Respond] = Command::Respond;
}

void CommandDispatcher::Dispatch(Packet& packet)
{
    if (packet.Size() < sizeof(uint32_t))
    {
        throw std::runtime_error("Packet too small to contain command ID");
    }

    Command::ID cmd = static_cast<Command::ID>(packet.Read<uint32_t>());
    auto it = handlers.find(cmd);
    if (it == handlers.end())
    {
        throw std::runtime_error("Unknown command ID");
    }

    auto handler = it->second;

    // Packetをムーブしてスレッド分離
    std::thread([handler, packet = std::move(packet)]() mutable
    {
        try
        {
            handler(packet);
        }
        catch (const std::exception& e)
        {
            WHBLogPrintf("Command handler exception: %s", e.what());
        }
        catch (...)
        {
            WHBLogPrintf("Unknown exception in handler");
        }
    }).detach();
}

