#include "commands/unknown_search.hpp"
#include "commands/common.hpp"

#include <format>

namespace Command
{
    void UnknownSearch(Packet& packet)
    {
        // プログレス通知の設定
        Memory::SetProgressCallback([packet](float progress)
        {
            Packet response(packet.GetContext());
            response.Write(std::to_underlying(Command::ID::UnknownSearch));
            response.Write(std::to_underlying(Command::Response::Progress));
            response.Write(progress);
            response.Send();
        });

        // パラメータの読み取り
        uint32_t startAddress, endAddress;
        ValueType valueType;
        if(!Memory::Search_LoadMeta(startAddress, endAddress, valueType))
        {
            throw "Failed to load search meta data";
        }
        Comparison comparison = static_cast<Comparison>(packet.Read<uint32_t>());

        switch (valueType)
        {
            case ValueType::Int8: Memory::UnknownSearch<int8_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::UInt8: Memory::UnknownSearch<uint8_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::Int16: Memory::UnknownSearch<int16_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::UInt16: Memory::UnknownSearch<uint16_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::Int32: Memory::UnknownSearch<int32_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::UInt32: Memory::UnknownSearch<uint32_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::Int64: Memory::UnknownSearch<int64_t>(startAddress, endAddress, valueType, comparison); break;
            case ValueType::UInt64: Memory::UnknownSearch<uint64_t>(startAddress, endAddress, valueType, comparison); break;
            default: throw std::runtime_error("Unknown value type");
        }

        // プログレス通知解除
        Memory::SetProgressCallback(nullptr);
    }
}