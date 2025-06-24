#include "commands/unknown_search.hpp"

#include <format>

namespace Command
{
    void UnknownSearch(Packet& packet)
    {
        // プログレス通知の設定
        Memory::SetProgressCallback([packet](float progress)
        {
            Packet response(packet.GetContext());
            response.Write<uint32_t>(std::to_underlying(Command::ID::UnknownSearch));
            response.Write<uint32_t>(std::to_underlying(Command::Response::Progress));
            response.Write<float>(progress);
            response.Send();
        });

        // パラメータの読み取り
        Comparison comp = static_cast<Comparison>(packet.Read<uint32_t>());
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>());
        uint32_t alignment = packet.Read<uint32_t>();

        switch (valueType)
        {
            case ValueType::Int8:
                Memory::UnknownSearch<int8_t>(comp, alignment, packet.Read<int8_t>());
                break;
            case ValueType::UInt8:
                Memory::UnknownSearch<uint8_t>(comp, alignment, packet.Read<uint8_t>());
                break;
            case ValueType::Int16:
                Memory::UnknownSearch<int16_t>(comp, alignment, packet.Read<int16_t>());
                break;
            case ValueType::UInt16:
                Memory::UnknownSearch<uint16_t>(comp, alignment, packet.Read<uint16_t>());
                break;
            case ValueType::Int32:
                Memory::UnknownSearch<int32_t>(comp, alignment, packet.Read<int32_t>());
                break;
            case ValueType::UInt32:
                Memory::UnknownSearch<uint32_t>(comp, alignment, packet.Read<uint32_t>());
                break;
            case ValueType::Int64:
                Memory::UnknownSearch<int64_t>(comp, alignment, packet.Read<int64_t>());
                break;
            case ValueType::UInt64:
                Memory::UnknownSearch<uint64_t>(comp, alignment, packet.Read<uint64_t>());
                break;
            default:
                throw std::runtime_error("Unknown value type");
        }

        // プログレス通知解除
        Memory::SetProgressCallback(nullptr);
    }
}