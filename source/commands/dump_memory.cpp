#include "commands/dump_memory.hpp"

// std
#include <format>
#include <regex>
#include <chrono>

// lz4
#include <lz4.h>

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
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>());
        bool compress = packet.Read<uint32_t>() != 0;

        Memory::Dump(start, size, valueType, compress);

        Memory::SetProgressCallback(nullptr);
    }
}

namespace Memory
{
    size_t g_originalValueSize = 0;
    size_t g_compressedValueSize = 0;
    size_t g_originalAddressSize = 0;
    size_t g_compressedAddressSize = 0;

    void WriteAddressData(std::ofstream& stream, const std::vector<uint32_t>& addresses)
    {
        size_t size = addresses.size() * sizeof(uint32_t);
        g_originalAddressSize += size;
        stream.write(reinterpret_cast<const char*>(addresses.data()), static_cast<std::streamsize>(size));
    }

    void WriteCompressedAddressData(std::ofstream& stream, const std::vector<uint32_t>& addresses, std::vector<int32_t>& deltas)
    {
        if (addresses.empty()) return;

        if (deltas.size() < addresses.size())
            deltas.resize(addresses.size());

        deltas[0] = static_cast<int32_t>(addresses[0]);
        for (size_t i = 1; i < addresses.size(); ++i)
            deltas[i] = static_cast<int32_t>(addresses[i]) - static_cast<int32_t>(addresses[i - 1]);

        const char* srcData = reinterpret_cast<const char*>(deltas.data());
        int srcSize = static_cast<int>(addresses.size() * sizeof(int32_t));
        g_originalAddressSize += srcSize;

        int maxDstSize = LZ4_compressBound(srcSize);
        static std::vector<char> tempBuffer;
        if (tempBuffer.size() < static_cast<size_t>(maxDstSize))
            tempBuffer.resize(maxDstSize);

        int compressedSize = LZ4_compress_default(srcData, tempBuffer.data(), srcSize, maxDstSize);
        if (compressedSize <= 0)
        {
            NotificationModule_AddErrorNotification("Failed to compress address data");
            return;
        }

        g_compressedAddressSize += compressedSize;
        stream.write(tempBuffer.data(), compressedSize);
    }

    void WriteValueData(std::ofstream& stream, const std::vector<uint8_t>& values)
    {
        g_originalValueSize += values.size();
        stream.write(reinterpret_cast<const char*>(values.data()), static_cast<std::streamsize>(values.size()));
    }

    void WriteCompressedValueData(std::ofstream& stream, const std::vector<uint8_t>& values, std::vector<char>& buffer)
    {
        if (values.empty()) return;

        const char* srcData = reinterpret_cast<const char*>(values.data());
        int srcSize = static_cast<int>(values.size());
        g_originalValueSize += srcSize;

        int maxDstSize = LZ4_compressBound(srcSize);
        if (static_cast<int>(buffer.size()) < maxDstSize)
            buffer.resize(maxDstSize);

        int compressedSize = LZ4_compress_default(srcData, buffer.data(), srcSize, maxDstSize);
        if (compressedSize <= 0)
        {
            NotificationModule_AddErrorNotification("Failed to compress value data");
            return;
        }

        g_compressedValueSize += compressedSize;
        stream.write(buffer.data(), compressedSize);
    }

    void Dump_SaveMetadata(uint32_t start, uint32_t size, Command::ValueType valueType, bool compressed)
    {
        std::ofstream meta(Memory::STARDUST_PATH / "Search" / "Search.meta");
        if (!meta)
        {
            NotificationModule_AddErrorNotification("Failed to write metadata file");
            return;
        }

        meta << std::format("start      = 0x{:08X}\n", start);
        meta << std::format("size       = 0x{:08X}\n", size);
        meta << std::format("compressed = {}\n", compressed ? "true" : "false");
        meta << std::format("valueType  = {}\n", ValueTypeToString(valueType));

        meta << std::format("originalValueSize   = {}\n", g_originalValueSize);
        meta << std::format("compressedValueSize = {}\n", g_compressedValueSize);
        meta << std::format("valueCompressionRate = {:.2f}%\n",
                            g_originalValueSize == 0 ? 0.0f :
                            100.0f * (static_cast<float>(g_compressedValueSize) / g_originalValueSize));

        meta << std::format("originalAddressSize   = {}\n", g_originalAddressSize);
        meta << std::format("compressedAddressSize = {}\n", g_compressedAddressSize);
        meta << std::format("addressCompressionRate = {:.2f}%\n",
                            g_originalAddressSize == 0 ? 0.0f :
                            100.0f * (static_cast<float>(g_compressedAddressSize) / g_originalAddressSize));

        meta.close();
    }

    void Dump(uint32_t start, uint32_t size, Command::ValueType valueType, bool compress)
    {
        const size_t valueSize = Command::GetValueSize(valueType);
        if (valueSize == 0)
        {
            NotificationModule_AddErrorNotification("Invalid ValueType");
            return;
        }

        const uint32_t alignment = static_cast<uint32_t>(valueSize);

        std::vector<uint8_t> valueBuffer;
        std::vector<uint32_t> addressBuffer;

        valueBuffer.reserve(THRESHOLD);
        addressBuffer.reserve(THRESHOLD / sizeof(uint32_t));

        std::filesystem::create_directories(Memory::STARDUST_PATH / "Search");
        std::ofstream valueStream(Memory::STARDUST_PATH / "Search" / "Value.bin", std::ios::binary);
        std::ofstream addressStream(Memory::STARDUST_PATH / "Search" / "Address.bin", std::ios::binary);

        std::vector<char> valueStreamBuffer(MB / 4), addressStreamBuffer(MB / 4);
        valueStream.rdbuf()->pubsetbuf(valueStreamBuffer.data(), valueStreamBuffer.size());
        addressStream.rdbuf()->pubsetbuf(addressStreamBuffer.data(), addressStreamBuffer.size());

        if (!valueStream || !addressStream)
        {
            NotificationModule_AddErrorNotification("Failed to open output files");
            return;
        }

        g_originalValueSize = g_compressedValueSize = 0;
        g_originalAddressSize = g_compressedAddressSize = 0;

        float lastProgressReported = -1.0f;

        std::vector<char> sharedValueBuffer;
        std::vector<int32_t> sharedAddressDeltas;

        auto lastProgressTime = Clock::now();

        uint64_t end = static_cast<uint64_t>(start) + size;
        for (uint32_t blockStart = start; blockStart < end; blockStart += BLOCK_SIZE)
        {
            uint64_t blockEnd = std::min(static_cast<uint64_t>(blockStart) + BLOCK_SIZE, end);

            if (OSIsAddressValid(blockStart) && OSIsAddressValid(blockEnd - 1))
            {
                for (uint32_t address = blockStart; address + valueSize <= blockEnd; address += alignment)
                {
                    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(address);
                    valueBuffer.insert(valueBuffer.end(), ptr, ptr + valueSize);
                    addressBuffer.push_back(address);

                    if (valueBuffer.size() >= THRESHOLD)
                    {
                        if (compress)
                        {
                            WriteCompressedValueData(valueStream, valueBuffer, sharedValueBuffer);
                        }
                        else
                        {
                            WriteValueData(valueStream, valueBuffer);
                        }
                        valueBuffer.clear();
                    }

                    if(addressBuffer.size() >= THRESHOLD / sizeof(uint32_t))
                    {
                        if (compress)
                        {
                            WriteCompressedAddressData(addressStream, addressBuffer, sharedAddressDeltas);
                        }
                        else
                        {
                            WriteAddressData(addressStream, addressBuffer);
                        }
                        addressBuffer.clear();
                    }

                    auto now = Clock::now();
                    if (progressCallback && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastProgressTime).count() >= 100)
                    {
                        float progress = static_cast<float>(address - start) / size;
                        progressCallback(progress);
                        lastProgressTime = now;
                    }
                }
            }
        }

        if (!valueBuffer.empty())
        {
            if (compress)
            {
                WriteCompressedValueData(valueStream, valueBuffer, sharedValueBuffer);
                WriteCompressedAddressData(addressStream, addressBuffer, sharedAddressDeltas);
            }
            else
            {
                WriteValueData(valueStream, valueBuffer);
                WriteAddressData(addressStream, addressBuffer);
            }
        }

        valueStream.close();
        addressStream.close();

        if (progressCallback && lastProgressReported < 1.0f)
        {
            progressCallback(1.0f);
        }

        Dump_SaveMetadata(start, size, valueType, compress);
    }

    void Dump_LoadMetadata(uint32_t& outStart, uint32_t& outSize, Command::ValueType& outValueType, bool& outCompressed)
    {
        std::ifstream meta(Memory::STARDUST_PATH / "Search" / "Search.meta");
        if (!meta)
        {
            NotificationModule_AddErrorNotification("Failed to open metadata file");
            return;
        }

        std::string line;
        std::regex hexPattern(R"((start|size)\s*=\s*0x([0-9A-Fa-f]+))");
        std::regex boolPattern(R"(compressed\s*=\s*(true|false))", std::regex::icase);
        std::regex valueTypePattern(R"(valueType\s*=\s*(\w+))");

        bool foundStart = false, foundSize = false, foundCompressed = false, foundValueType = false;

        while (std::getline(meta, line))
        {
            std::smatch match;
            if (std::regex_search(line, match, hexPattern))
            {
                std::string key = match[1];
                uint32_t value = std::stoul(match[2], nullptr, 16);
                if (key == "start")
                {
                    outStart = value;
                    foundStart = true;
                }
                else if (key == "size")
                {
                    outSize = value;
                    foundSize = true;
                }
            }
            else if (std::regex_search(line, match, boolPattern))
            {
                std::string val = match[1];
                outCompressed = (val == "true");
                foundCompressed = true;
            }
            else if (std::regex_search(line, match, valueTypePattern))
            {
                std::string val = match[1];
                if (StringToValueType(val, outValueType))
                {
                    foundValueType = true;
                }
                else
                {
                    NotificationModule_AddErrorNotification("Unknown valueType in metadata");
                    return;
                }
            }
        }

        meta.close();

        if (!foundStart || !foundSize)
        {
            NotificationModule_AddErrorNotification("Metadata file is missing required values");
            return;
        }

        if (!foundCompressed)
        {
            outCompressed = false;
        }

        if (!foundValueType)
        {
            outValueType = Command::ValueType::UInt8; // デフォルト値
        }
    }
}
