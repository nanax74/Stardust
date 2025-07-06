#include "commands/write_memory.hpp"
#include "commands/common.hpp"

#include <format>

namespace Command
{
    void WriteMemory(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ID::WriteMemory));

        if (packet.Size() < sizeof(uint32_t) * 2)
        {
            response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
            response.Send();
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
                if (packet.Size() < sizeof(uint8_t))
                {
                    response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                    response.Send();
                    throw std::runtime_error("Packet too small for uint8_t value");
                }
                uint8_t value = packet.Read<uint8_t>();
                Memory::Write(address, value);
                break;
            }
            // 2byte
            case ValueType::Int16:
            case ValueType::UInt16:
            {
                if (packet.Size() < sizeof(uint16_t))
                {
                    response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                    response.Send();
                    throw std::runtime_error("Packet too small for uint16_t value");
                }
                uint16_t value = packet.Read<uint16_t>();
                Memory::Write(address, value);
                break;
            }
            // 4byte
            case ValueType::Int32:
            case ValueType::UInt32:
            case ValueType::Float:
            {
                if (packet.Size() < sizeof(uint32_t))
                {
                    response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                    response.Send();
                    throw std::runtime_error("Packet too small for uint32_t value");
                }
                uint32_t value = packet.Read<uint32_t>();
                Memory::Write(address, value);
                break;
            }
            // 8byte
            case ValueType::Int64:
            case ValueType::UInt64:
            case ValueType::Double:
            {
                if (packet.Size() < sizeof(uint64_t))
                {
                    response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                    response.Send();
                    throw std::runtime_error("Packet too small for uint64_t value");
                }
                uint64_t value = packet.Read<uint64_t>();
                Memory::Write(address, value);
                break;
            }
            default:
                response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                response.Send();
                throw std::runtime_error("Unsupported value type");
        }

        // 成功応答
        response.Write<uint32_t>(std::to_underlying(Response::Success)); // 成功
        response.Send();
    }
}