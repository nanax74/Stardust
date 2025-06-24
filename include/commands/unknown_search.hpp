#pragma once

#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

#include <notifications/notifications.h>

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
        /*
        std::ifstream ifs(Memory::STARDUST_PATH / "Search" / "Dump.bin", std::ios::binary);
        if (!ifs)
        {
            NotificationModule_AddErrorNotification("Failed to open memory dump file");
            return;
        }

        uint32_t start = 0;
        uint32_t size = 0;
        if (!Dump_LoadMetadata(start, size))
        {
            NotificationModule_AddErrorNotification("Failed to load memory dump metadata");
            return;
        }

        std::vector<uint8_t> buffer(size);
        ifs.read(reinterpret_cast<char*>(buffer.data()), size);
        ifs.close();

        std::vector<uint32_t> results;
        size_t typeSize = sizeof(T);

        float lastProgress = -1.0f;
        for (size_t offset = 0; offset + typeSize <= size; offset += alignment)
        {
            T currentValue;
            std::memcpy(&currentValue, &buffer[offset], typeSize);

            // 比較処理（簡易版）
            bool matched = false;
            switch (comparison)
            {
                case Command::Comparison::Equal: matched = (currentValue == value); break;
                case Command::Comparison::NotEqual: matched = (currentValue != value); break;
                case Command::Comparison::LessThan: matched = (currentValue <  value); break;
                case Command::Comparison::LessOrEqual: matched = (currentValue <= value); break;
                case Command::Comparison::GreaterThan: matched = (currentValue >  value); break;
                case Command::Comparison::GreaterOrEqual: matched = (currentValue >= value); break;
                default: break;
            }

            if (matched)
            {
                results.push_back(start + static_cast<uint32_t>(offset));
            }

            // 進捗の通知（1%ごと）
            if (progressCallback)
            {
                float progress = static_cast<float>(offset) / size;
                if (progress - lastProgress >= 0.01f)
                {
                    progressCallback(progress);
                    lastProgress = progress;
                }
            }
        }

        if (progressCallback && lastProgress < 1.0f)
        {
            progressCallback(1.0f);
        }

        // 結果を保存
        Memory::SetSearchResults(results);
        */
    }
}
