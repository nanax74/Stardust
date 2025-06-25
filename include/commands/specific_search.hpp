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
    void Search(uint32_t start, uint32_t size, const T& value, uint32_t alignment = alignof(T))
    {
        
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");

        std::filesystem::create_directories(Memory::STARDUST_PATH / "Search");
        std::ofstream ofs(Memory::STARDUST_PATH / "Search" / "results.bin", std::ios::binary);
        if (!ofs)
        {
            NotificationModule_AddErrorNotification("Failed to open output file");
            return;
        }
    
        float lastProgressReported = -1.0f;
        uint32_t end = start + size;
    
        for (uint32_t address = start; address < end; address += alignment)
        {
            // アラインメントはこの段階で担保されている
            T temp;
            std::memcpy(&temp, reinterpret_cast<const void*>(address), sizeof(T));
        
            if (temp == value)
            {
                ofs.write(reinterpret_cast<const char*>(&address), sizeof(address));
            }
        
            float progress = static_cast<float>(address - start) / size;
            if (g_ReportProgress && progress - lastProgressReported >= 0.01f)
            {
                g_ReportProgress(progress);
                lastProgressReported = progress;
            }
        }
    
        if (g_ReportProgress && lastProgressReported < 1.0f)
        {
            g_ReportProgress(1.0f);
        }
    }
}
