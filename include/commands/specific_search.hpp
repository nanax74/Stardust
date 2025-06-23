#pragma once

#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

#include <notifications/notifications.h>

#include "packet.hpp"


namespace Command
{
    // 特定の値を検索するコマンド
    void SpecificSearch(Packet& packet);
}

namespace Memory
{
    template <typename T>
    void Search(uint32_t start, uint32_t end, const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");

        std::filesystem::create_directories(Memory::STARDUST_PATH / "Search");

        std::ofstream ofs(Memory::STARDUST_PATH / "Search" / "results.bin", std::ios::binary);
        if (!ofs)
        {
            NotificationModule_AddErrorNotification("Failed to open output file");
            return;
        }

        uint32_t total = end - start;
        float lastProgressReported = -1.0f;  // -1で初期化し、必ず最初の進捗は報告されるように

        for (uint32_t address = start; address < end; address += sizeof(T))
        {
            T* ptr = reinterpret_cast<T*>(address);
            float progress = total > 0 ? static_cast<float>(address - start) / total : 0.0f;

            if (*ptr == value)
            {
                ofs.write(reinterpret_cast<const char*>(&address), sizeof(address));
            }

            // 1% (0.01) 以上進捗が進んだらコールバック呼び出し
            if (progressCallback && progress - lastProgressReported >= 0.01f)
            {
                progressCallback(address, progress);
                lastProgressReported = progress;
            }
        }
    }
}
