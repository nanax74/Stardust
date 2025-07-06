#include "commands/specific_search.hpp"
#include "commands/common.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>
#include <memory> // For std::shared_ptr

#include <whb/log.h>

namespace Command
{
    void SpecificSearch(Packet& packet)
    {
        // パケットから検索パラメータを読み取る
        uint32_t startAddress = packet.Read<uint32_t>();
        uint32_t endAddress = packet.Read<uint32_t>();
        Command::ValueType valueType = static_cast<Command::ValueType>(packet.Read<uint32_t>());
        Command::Comparison comparisonOperator = static_cast<Command::Comparison>(packet.Read<uint32_t>());

        // 検索値を読み取る (ValueType に応じて)
        // Correctly read the search value based on its type
        std::shared_ptr<SocketContext> ctx = packet.GetContext();

        switch (valueType)
        {
            case ValueType::Int8:   Memory::SpecificSearch<int8_t>(startAddress, endAddress, comparisonOperator, packet.Read<int8_t>(), ctx); break;
            case ValueType::UInt8:  Memory::SpecificSearch<uint8_t>(startAddress, endAddress, comparisonOperator, packet.Read<uint8_t>(), ctx); break;
            case ValueType::Int16:  Memory::SpecificSearch<int16_t>(startAddress, endAddress, comparisonOperator, packet.Read<int16_t>(), ctx); break;
            case ValueType::UInt16: Memory::SpecificSearch<uint16_t>(startAddress, endAddress, comparisonOperator, packet.Read<uint16_t>(), ctx); break;
            case ValueType::Int32:  Memory::SpecificSearch<int32_t>(startAddress, endAddress, comparisonOperator, packet.Read<int32_t>(), ctx); break;
            case ValueType::UInt32: Memory::SpecificSearch<uint32_t>(startAddress, endAddress, comparisonOperator, packet.Read<uint32_t>(), ctx); break;
            case ValueType::Int64:  Memory::SpecificSearch<int64_t>(startAddress, endAddress, comparisonOperator, packet.Read<int64_t>(), ctx); break;
            case ValueType::UInt64: Memory::SpecificSearch<uint64_t>(startAddress, endAddress, comparisonOperator, packet.Read<uint64_t>(), ctx); break;
            case ValueType::Float:  Memory::SpecificSearch<float>(startAddress, endAddress, comparisonOperator, packet.Read<float>(), ctx); break;
            case ValueType::Double: Memory::SpecificSearch<double>(startAddress, endAddress, comparisonOperator, packet.Read<double>(), ctx); break;
            default: throw std::runtime_error("Unsupported ValueType for SpecificSearch");
        }

        // 検索完了の応答
        Packet response(ctx);
        response.Write(ID::SpecificSearch);
        response.Write(Command::Response::Success);
        response.Send();
    }
}

namespace Memory
{
    template <typename T>
    void SpecificSearch(uint32_t startAddress, uint32_t endAddress, Command::Comparison comparison, T searchValue, std::shared_ptr<SocketContext> ctx)
    {
        WHBLogPrintf("Memory::SpecificSearch: Start=0x%X, End=0x%X, Comp=%u, Value=%f (float example)",
                     startAddress, endAddress, static_cast<uint32_t>(comparison), static_cast<float>(searchValue)); // Log for debugging

        // メタデータをロード
        uint32_t metaStartAddress, metaEndAddress;
        Command::ValueType metaValueType;
        if (!Memory::Search_LoadMeta(metaStartAddress, metaEndAddress, metaValueType))
        {
            throw std::runtime_error("Failed to load search meta data");
        }

        // Check if the dumped data type matches the search type
        if (metaValueType != Command::GetValueType<T>())
        {
            throw std::runtime_error("ValueType mismatch between search and dumped data");
        }

        namespace fs = std::filesystem;
        fs::path basePath = fs::path(Memory::STARDUST_PATH) / "Search";
        std::ifstream valueFile(basePath / "Value.bin", std::ios::binary);
        std::ifstream addressFile(basePath / "Address.bin", std::ios::binary);

        if (!valueFile.is_open() || !addressFile.is_open())
        {
            throw std::runtime_error("Failed to open Value.bin or Address.bin");
        }

        size_t typeSize = Command::GetTypeSize(Command::GetValueType<T>());
        size_t numEntries = (metaEndAddress - metaStartAddress) / typeSize;

        // Seek to the correct starting position in the files based on startAddress
        // Assuming the files are ordered by address and start from metaStartAddress
        size_t startIndex = (startAddress - metaStartAddress) / typeSize;
        if (startIndex > numEntries) startIndex = numEntries; // Cap at numEntries

        valueFile.seekg(startIndex * typeSize, std::ios::beg);
        addressFile.seekg(startIndex * sizeof(uint32_t), std::ios::beg);

        // Progress reporting
        auto lastReport = std::chrono::steady_clock::now();
        size_t processedEntries = 0;

        for (size_t i = startIndex; i < numEntries; ++i)
        {
            if (!valueFile.good() || !addressFile.good())
            {
                WHBLogPrintf("Error reading search files at entry %zu", i);
                break;
            }

            T currentValue;
            uint32_t currentAddress;

            valueFile.read(reinterpret_cast<char*>(&currentValue), typeSize);
            addressFile.read(reinterpret_cast<char*>(&currentAddress), sizeof(uint32_t));

            // Check if the current address is within the requested range
            if (currentAddress >= startAddress && currentAddress < endAddress)
            {
                if (Command::CompareValues(currentValue, searchValue, comparison))
                {
                    // Match found, send to Vortex
                    Memory::SendSearchResult<T>(currentAddress, currentValue, Command::ID::SpecificSearch, ctx);
                }
            }

            processedEntries++;

            // Progress reporting
            auto now = std::chrono::steady_clock::now();
            if (now - lastReport >= std::chrono::milliseconds(100))
            {
                float progress = static_cast<float>(processedEntries) / (numEntries - startIndex);
                Packet response(ctx);
                response.Write(Command::ID::SpecificSearch);
                response.Write(Command::Response::Progress);
                response.Write(progress);
                response.Send();
                lastReport = now;
            }
        }

        // Final progress update to 1.0f
        Packet finalProgressResponse(ctx);
        finalProgressResponse.Write(Command::ID::SpecificSearch);
        finalProgressResponse.Write(Command::Response::Progress);
        finalProgressResponse.Write(1.0f);
        finalProgressResponse.Send();

        valueFile.close();
        addressFile.close();
    }
}