#include "commands/write_memory.hpp"
#include "commands/common.hpp"

#include <format>

namespace Command
{
    void WriteMemory::Process(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write(ClientCommand::WriteMemory);

        if (packet.Size() < sizeof(uint32_t) * 2)
        {
            throw std::runtime_error("Packet too small to contain type and address");
        }

        uint32_t address = packet.Read<uint32_t>();
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>());

        switch (valueType)
        {
            // 1byte
            case ValueType::Int8:
            case ValueType::UInt8:
            {
                uint8_t value = packet.Read<uint8_t>();
                Memory::Write(address, value);
                break;
            }
            // 2byte
            case ValueType::Int16:
            case ValueType::UInt16:
            {
                uint16_t value = packet.Read<uint16_t>();
                Memory::Write(address, value);
                break;
            }
            // 4byte
            case ValueType::Int32:
            case ValueType::UInt32:
            case ValueType::Float:
            {
                uint32_t value = packet.Read<uint32_t>();
                Memory::Write(address, value);
                break;
            }
            // 8byte
            case ValueType::Int64:
            case ValueType::UInt64:
            case ValueType::Double:
            {
                uint64_t value = packet.Read<uint64_t>();
                Memory::Write(address, value);
                break;
            }
            default:
                throw std::runtime_error("Unsupported value type");
        }

        // 成功応答
        response.Write(Response::Success);
        response.Send();
    }
}