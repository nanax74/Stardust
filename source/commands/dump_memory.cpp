#include "commands/dump_memory.hpp"

#include <format>

#include <coreinit/memorymap.h>

namespace Command
{
    void DumpMemory(Packet& packet)
    {
        // 例: 進捗を受け取るラムダ関数を登録
        Memory::SetProgressCallback([packet](float progress)
        {
            Packet response(packet.GetContext());
            response.Write<uint32_t>(std::to_underlying(Command::ID::DumpMemory));
            response.Write<uint32_t>(std::to_underlying(Command::Response::Progress));
            response.Write<float>(progress);
            response.Send();
        });

        // packet からダンプ範囲やサイズを取得
        uint32_t start = packet.Read<uint32_t>();
        uint32_t size = packet.Read<uint32_t>();

        // サーチを実行
        Memory::Dump(start, size);

        // 検索終了後にコールバックをクリア
        Memory::SetProgressCallback(nullptr);
    }
}

namespace Memory
{
    void Dump(uint32_t start, uint32_t size)
    {
        constexpr size_t FLUSH_THRESHOLD = MB; // 1MB
        std::vector<uint8_t> writeBuffer;

        std::filesystem::create_directories(Memory::STARDUST_PATH / "Dump");
        std::ofstream ofs(Memory::STARDUST_PATH / "Dump" / "memory_dump.bin", std::ios::binary);
        if (!ofs)
        {
            NotificationModule_AddErrorNotification("Failed to open output file");
            return;
        }

        float lastProgressReported = -1.0f;
        uint64_t end = static_cast<uint64_t>(start) + size;
        uint32_t address = start;

        while (static_cast<uint64_t>(address) + sizeof(uint64_t) <= end)
        {
            if (!OSIsAddressValid(address))
            {
                std::string msg = std::format("Invalid address at 0x{:08X}", address);
                NotificationModule_AddErrorNotification(msg.c_str());
                ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
                ofs.close();
                return;
            }

            uint64_t value;
            std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
            writeBuffer.insert(writeBuffer.end(), bytes, bytes + sizeof(value));
            address += sizeof(uint64_t);

            if (writeBuffer.size() >= FLUSH_THRESHOLD)
            {
                ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
                writeBuffer.clear();
            }

            float progress = static_cast<float>(address - start) / size;
            if (progressCallback && progress - lastProgressReported >= 0.01f)
            {
                progressCallback(progress);
                lastProgressReported = progress;
            }
        }

        while (static_cast<uint64_t>(address) < end)
        {
            if (!OSIsAddressValid(address))
            {
                NotificationModule_AddErrorNotification(std::format("Invalid address at 0x{:08X}", address).c_str());
                ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
                ofs.close();
                return;
            }

            size_t remaining = end - address;

            if (remaining >= 4)
            {
                uint32_t value;
                std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
                const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                writeBuffer.insert(writeBuffer.end(), bytes, bytes + sizeof(value));
                address += 4;
            }
            else if (remaining >= 2)
            {
                uint16_t value;
                std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
                const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
                writeBuffer.insert(writeBuffer.end(), bytes, bytes + sizeof(value));
                address += 2;
            }
            else
            {
                uint8_t value;
                std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof(value));
                writeBuffer.push_back(value);
                address += 1;
            }

            if (writeBuffer.size() >= FLUSH_THRESHOLD)
            {
                ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
                writeBuffer.clear();
            }

            float progress = static_cast<float>(address - start) / size;
            if (progressCallback && progress - lastProgressReported >= 0.01f)
            {
                progressCallback(progress);
                lastProgressReported = progress;
            }
        }

        // 最後に残った分を書き出す
        if (!writeBuffer.empty())
        {
            ofs.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
        }

        if (progressCallback && lastProgressReported < 1.0f)
        {
            progressCallback(1.0f);
        }

        ofs.close();
    }
}