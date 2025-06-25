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

        uint32_t startAddress = packet.Read<uint32_t>();
        uint32_t endAddress = packet.Read<uint32_t>();
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>());

        switch(valueType)
        {
            case ValueType::Int8: Memory::Dump<int8_t>(startAddress, endAddress); break;
            case ValueType::UInt8: Memory::Dump<uint8_t>(startAddress, endAddress); break;
            case ValueType::Int16: Memory::Dump<int16_t>(startAddress, endAddress); break;
            case ValueType::UInt16: Memory::Dump<uint16_t>(startAddress, endAddress); break;
            case ValueType::Int32: Memory::Dump<int32_t>(startAddress, endAddress); break;
            case ValueType::UInt32: Memory::Dump<uint32_t>(startAddress, endAddress); break;
            case ValueType::Int64: Memory::Dump<int64_t>(startAddress, endAddress); break;
            case ValueType::UInt64: Memory::Dump<uint64_t>(startAddress, endAddress); break;
            case ValueType::Float: Memory::Dump<float>(startAddress, endAddress); break;
            case ValueType::Double: Memory::Dump<double>(startAddress, endAddress); break;
            default: throw std::runtime_error("Unsupported ValueType");
        }

        // メタデータ保存
        if(!Memory::Search_SaveMeta(startAddress, endAddress, valueType))
        {
            throw std::runtime_error("Failed to save search meta data");
        }

        Memory::SetProgressCallback(nullptr);
    }
}
