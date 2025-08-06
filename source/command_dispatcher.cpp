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
#include "commands/get_search_results.hpp"
#include "commands/get_search_results_count.hpp"
#include "commands/respond.hpp"

#include "commands/subscribe_address.hpp"
#include "commands/unsubscribe_address.hpp"

std::unordered_map<Command::ClientCommand, std::shared_ptr<Command::Base>> handlers;

namespace CommandDispatcher
{
    void RegisterHandlers()
    {
        // メモリ書き込み
        handlers[Command::ClientCommand::WriteMemory] = std::make_shared<Command::WriteMemory>();

        // メモリ読み込み
        handlers[Command::ClientCommand::ReadMemory] = std::make_shared<Command::ReadMemory>();

        // 複数メモリ読み込み
        handlers[Command::ClientCommand::ReadMultipleMemory] = std::make_shared<Command::ReadMultipleMemory>();

        // メモリダンプ
        handlers[Command::ClientCommand::DumpForSearch] = std::make_shared<Command::DumpForSearch>();

        // 既知の値検索
        handlers[Command::ClientCommand::SpecificSearch] = std::make_shared<Command::SpecificSearch>();

        // 未知の値検索
        handlers[Command::ClientCommand::UnknownSearch] = std::make_shared<Command::UnknownSearch>();

        // サーチ関係のファイルを消去
        handlers[Command::ClientCommand::ClearSearchFile] = std::make_shared<Command::ClearSearchFile>();

        // サーチ関係のファイルの有無を確認
        handlers[Command::ClientCommand::CheckSearchFile] = std::make_shared<Command::CheckSearchFile>();

        // サーチ結果を取得
        handlers[Command::ClientCommand::GetSearchResults] = std::make_shared<Command::GetSearchResults>();

        // サーチ結果の数を取得
        handlers[Command::ClientCommand::GetSearchResultsCount] = std::make_shared<Command::GetSearchResultsCount>();

        // 応答を返すだけ
        handlers[Command::ClientCommand::Respond] = std::make_shared<Command::Respond>();
        
        // 監視アドレスを登録
        handlers[Command::ClientCommand::SubscribeAddress] = std::make_shared<Command::SubscribeAddress>();

        // 監視アドレスの登録を解除
        handlers[Command::ClientCommand::UnsubscribeAddress] = std::make_shared<Command::UnsubscribeAddress>();
    }

    void Dispatch(Packet& packet)
    {
        Log::Printf("{}", packet.ToString());
        if (packet.Size() < sizeof(uint32_t))
        {
            throw std::runtime_error("Packet too small to contain command ID");
        }

        Command::ClientCommand cmd = static_cast<Command::ClientCommand>(packet.Read<uint32_t>());
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
                Log::Printf("Command handler exception: {}", e.what());
            }
            catch (...)
            {
                Log::Printf("Unknown exception in handler");
            }
        }).detach();
    }
}