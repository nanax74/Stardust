#include "logger.hpp"

#include <format>
#include <cstring> // memcpy
#include <thread>
#include <memory>

#include "notification.hpp"

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

std::unordered_map<Command::ID, std::shared_ptr<Command::Base>> handlers;

namespace CommandDispatcher
{
    void RegisterHandlers()
    {
        // メモリ書き込み
        handlers[Command::ID::WriteMemory] = std::make_shared<Command::WriteMemory>();

        // メモリ読み込み
        handlers[Command::ID::ReadMemory] = std::make_shared<Command::ReadMemory>();

        // 複数メモリ読み込み
        handlers[Command::ID::ReadMultipleMemory] = std::make_shared<Command::ReadMultipleMemory>();

        // メモリダンプ
        handlers[Command::ID::DumpForSearch] = std::make_shared<Command::DumpForSearch>();

        // 既知の値検索
        handlers[Command::ID::SpecificSearch] = std::make_shared<Command::SpecificSearch>();

        // 未知の値検索
        handlers[Command::ID::UnknownSearch] = std::make_shared<Command::UnknownSearch>();

        // サーチ関係のファイルを消去
        handlers[Command::ID::ClearSearchFile] = std::make_shared<Command::ClearSearchFile>();

        // サーチ関係のファイルの有無を確認
        handlers[Command::ID::CheckSearchFile] = std::make_shared<Command::CheckSearchFile>();

        // 応答を返すだけ
        handlers[Command::ID::Respond] = std::make_shared<Command::Respond>();
    }

    void Dispatch(Packet& packet)
    {
        Log::Printf("{}", packet.ToString());
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
                handler->Execute(packet);
            }
            catch (const std::exception& e)
            {
                Log::Printf("Command handler exception: %s", e.what());
            }
            catch (...)
            {
                Log::Printf("Unknown exception in handler");
            }
        }).detach();
    }
}