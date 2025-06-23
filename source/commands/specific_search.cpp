#include "commands/specific_search.hpp"

#include <format>

#include <notifications/notifications.h>

namespace Command
{
    void SpecificSearch(Packet& packet)
    {
        // 例: 進捗を受け取るラムダ関数を登録
        Memory::SetProgressCallback([](uint32_t addr, float progress)
        {
            // 進捗を何かに送信する、ログに出すなど
            std::string msg = std::format("Progress: 0x{:08X} ({:.2f}%)\n", addr, progress * 100.0f);
            NotificationModule_AddInfoNotification(msg.c_str());
        });

        // packet から検索範囲や値を取得する例
        uint32_t start = packet.Read<uint32_t>();
        uint32_t end = packet.Read<uint32_t>();
        int value = packet.Read<int>();

        Memory::Search(start, end, value);

        // 検索終了後にコールバックをクリア
        Memory::SetProgressCallback(nullptr);
    }
}