#include "commands/dump_memory.hpp"
#include "commands/common.hpp"

// std
#include <format>
#include <regex>
#include <chrono>

// wut
#include <coreinit/memorymap.h>

using Clock = std::chrono::steady_clock;

namespace Command
{
    void DumpMemory(Packet& packet)
    {
        Memory::SetProgressCallback([packet](float progress)
        {
            Packet response(packet.GetContext());
            response.Write<uint32_t>(std::to_underlying(Command::ID::DumpMemory));
            response.Write<uint32_t>(std::to_underlying(Command::Response::Progress));
            response.Write<float>(progress);
            response.Send();
        });

        uint32_t start = packet.Read<uint32_t>();
        uint32_t size = packet.Read<uint32_t>();
        ValueType valType = static_cast<ValueType>(packet.Read<uint32_t>());
        bool comp = packet.Read<uint32_t>() != 0;

        Memory::Dump(start, size, valType, comp);

        Memory::SetProgressCallback(nullptr);
    }
}

namespace Memory
{
    namespace fs = std::filesystem;

    void Dump(uint32_t startAddress, uint32_t dumpSize, Command::ValueType valueType, bool compress)
    {
        fs::create_directories(fs::path(STARDUST_PATH) / "Search");

        std::ofstream valueFile((fs::path(STARDUST_PATH) / "Search" / "Value.bin").string(), std::ios::binary);
        std::ofstream addressFile((fs::path(STARDUST_PATH) / "Search" / "Address.bin").string(), std::ios::binary);

        std::vector<uint8_t> valueBuffer;
        std::vector<uint32_t> addressBuffer;

        auto lastReport = std::chrono::steady_clock::now();

        uint64_t endAddress = static_cast<uint64_t>(startAddress) + dumpSize;
        for (uint32_t addr = startAddress; static_cast<uint64_t>(addr) < endAddress;)
        {
            uint32_t blockStart = addr;
            uint32_t blockEnd = std::min(addr + BLOCK_SIZE, startAddress + dumpSize);

            if (!OSIsAddressValid(blockStart) || !OSIsAddressValid(blockEnd - 1))
            {
                addr = blockEnd;
                continue;
            }

            for (; addr < blockEnd; )
            {
                if (!OSIsAddressValid(addr))
                {
                    ++addr;
                    continue;
                }

                size_t typeSize = GetTypeSize(valueType);

                uint8_t buffer[8] = {};
                memcpy(buffer, reinterpret_cast<void*>(addr), typeSize);

                valueBuffer.insert(valueBuffer.end(), buffer, buffer + typeSize);
                addressBuffer.push_back(addr);
                addr += typeSize;

                auto now = std::chrono::steady_clock::now();
                if (now - lastReport >= std::chrono::milliseconds(100))
                {
                    float progress = static_cast<float>(addr - startAddress) / dumpSize;
                    if (g_ReportProgress) g_ReportProgress(progress);
                    lastReport = now;
                }

                if (valueBuffer.size() >= THRESHOLD)
                {
                    if (compress)
                    {
                        auto compressedValues = CompressValues(valueBuffer.data(), valueBuffer.size(), GetTypeSize(valueType));
                        valueFile.write(reinterpret_cast<const char*>(compressedValues.data()), compressedValues.size());

                        auto compressedAddresses = CompressAddresses(addressBuffer);
                        addressFile.write(reinterpret_cast<const char*>(compressedAddresses.data()), compressedAddresses.size());
                    }
                    else
                    {
                        valueFile.write(reinterpret_cast<const char*>(valueBuffer.data()), valueBuffer.size());
                        addressFile.write(reinterpret_cast<const char*>(addressBuffer.data()), addressBuffer.size() * sizeof(uint32_t));
                    }

                    valueBuffer.clear();
                    addressBuffer.clear();
                }
            }
        }

        // 残りのバッファを書き込み
        if (!valueBuffer.empty())
        {
            if (compress)
            {
                auto compressedValues = CompressValues(valueBuffer.data(), valueBuffer.size(), GetTypeSize(valueType));
                auto compressedAddrs  = CompressAddresses(addressBuffer);
                valueFile.write(reinterpret_cast<const char*>(compressedValues.data()), compressedValues.size());
                addressFile.write(reinterpret_cast<const char*>(compressedAddrs.data()), compressedAddrs.size() * sizeof(uint32_t));
            }
            else
            {
                valueFile.write(reinterpret_cast<const char*>(valueBuffer.data()), valueBuffer.size());
                addressFile.write(reinterpret_cast<const char*>(addressBuffer.data()), addressBuffer.size() * sizeof(uint32_t));
            }
        }

        valueFile.close();
        addressFile.close();

        // メタデータ保存
        Search_SaveMeta(startAddress, dumpSize, valueType, compress);
    }
}
