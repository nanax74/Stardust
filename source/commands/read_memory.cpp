#include "commands/read_memory.hpp"
#include "commands/common.hpp"

namespace Command
{
    void ReadMemory::Process(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ID::ReadMemory));

        if (packet.Size() < sizeof(uint32_t) * 2)
        {
            response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
            response.Send();
            throw std::runtime_error("Packet too small to contain address and value type");
        }

        uint32_t address = packet.Read<uint32_t>();
        Command::ValueType valueType = static_cast<Command::ValueType>(packet.Read<uint32_t>());

        response.Write<uint32_t>(std::to_underlying(Response::Data)); // 成功応答
        response.Write(address); // アドレスを返す

        // ValueType に応じてメモリを読み込み、Packet に書き込む
        switch (valueType)
        {
            case Command::ValueType::Int8:   response.Write(Memory::Read<int8_t>(address)); break;
            case Command::ValueType::UInt8:  response.Write(Memory::Read<uint8_t>(address)); break;
            case Command::ValueType::Int16:  response.Write(Memory::Read<int16_t>(address)); break;
            case Command::ValueType::UInt16: response.Write(Memory::Read<uint16_t>(address)); break;
            case Command::ValueType::Int32:  response.Write(Memory::Read<int32_t>(address)); break;
            case Command::ValueType::UInt32: response.Write(Memory::Read<uint32_t>(address)); break;
            case Command::ValueType::Int64:  response.Write(Memory::Read<int64_t>(address)); break;
            case Command::ValueType::UInt64: response.Write(Memory::Read<uint64_t>(address)); break;
            case Command::ValueType::Float:  response.Write(Memory::Read<float>(address)); break;
            case Command::ValueType::Double: response.Write(Memory::Read<double>(address)); break;
            default:
                response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
                response.Send();
                throw std::runtime_error("Unsupported value type for ReadMemory");
        }
        response.Send();
    }
}