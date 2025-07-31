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
    void SpecificSearch::Process(Packet& packet)
    {
        // パラメーターの取得
        uint32_t startAddress, endAddress;
        ValueType valueType;
        if(!Memory::Search_LoadMeta(startAddress, endAddress, valueType))
        {
            throw "Failed to load search meta data";
        }
        Command::Comparison comparison = static_cast<Command::Comparison>(packet.Read<uint32_t>());

        // 検索値を読み取る (ValueType に応じて)
        // Correctly read the search value based on its type
        std::shared_ptr<SocketContext> ctx = packet.GetContext();

        switch (valueType)
        {
            case ValueType::Int8:   Memory::SpecificSearch<int8_t>(startAddress, endAddress, comparison, packet.Read<int8_t>(), ctx); break;
            case ValueType::UInt8:  Memory::SpecificSearch<uint8_t>(startAddress, endAddress, comparison, packet.Read<uint8_t>(), ctx); break;
            case ValueType::Int16:  Memory::SpecificSearch<int16_t>(startAddress, endAddress, comparison, packet.Read<int16_t>(), ctx); break;
            case ValueType::UInt16: Memory::SpecificSearch<uint16_t>(startAddress, endAddress, comparison, packet.Read<uint16_t>(), ctx); break;
            case ValueType::Int32:  Memory::SpecificSearch<int32_t>(startAddress, endAddress, comparison, packet.Read<int32_t>(), ctx); break;
            case ValueType::UInt32: Memory::SpecificSearch<uint32_t>(startAddress, endAddress, comparison, packet.Read<uint32_t>(), ctx); break;
            case ValueType::Int64:  Memory::SpecificSearch<int64_t>(startAddress, endAddress, comparison, packet.Read<int64_t>(), ctx); break;
            case ValueType::UInt64: Memory::SpecificSearch<uint64_t>(startAddress, endAddress, comparison, packet.Read<uint64_t>(), ctx); break;
            case ValueType::Float:  Memory::SpecificSearch<float>(startAddress, endAddress, comparison, packet.Read<float>(), ctx); break;
            case ValueType::Double: Memory::SpecificSearch<double>(startAddress, endAddress, comparison, packet.Read<double>(), ctx); break;
            default: throw std::runtime_error("Unsupported ValueType for SpecificSearch");
        }

        // 検索完了の応答
        Packet response(ctx);
        response.Write(ID::SpecificSearch);
        response.Write(Command::Response::Success);
        response.Send();
    }
}
