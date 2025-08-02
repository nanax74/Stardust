#include "commands/unknown_search.hpp"
#include "commands/common.hpp"

#include <format>

namespace Command
{
    void UnknownSearch::Process(Packet& packet)
    {
        auto context = packet.GetContext();

        // 進捗通知の設定
        Memory::SetProgressCallback([context](float progress)
        {
            Packet response(context);
            response.Write(Command::ID::UnknownSearch);
            response.Write(Command::Response::Progress);
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

        try
        {
            switch (valueType)
            {
                case ValueType::Int8   : Memory::UnknownSearch<int8_t>  (startAddress, endAddress, comparison); break;
                case ValueType::UInt8  : Memory::UnknownSearch<uint8_t> (startAddress, endAddress, comparison); break;
                case ValueType::Int16  : Memory::UnknownSearch<int16_t> (startAddress, endAddress, comparison); break;
                case ValueType::UInt16 : Memory::UnknownSearch<uint16_t>(startAddress, endAddress, comparison); break;
                case ValueType::Int32  : Memory::UnknownSearch<int32_t> (startAddress, endAddress, comparison); break;
                case ValueType::UInt32 : Memory::UnknownSearch<uint32_t>(startAddress, endAddress, comparison); break;
                case ValueType::Int64  : Memory::UnknownSearch<int64_t> (startAddress, endAddress, comparison); break;
                case ValueType::UInt64 : Memory::UnknownSearch<uint64_t>(startAddress, endAddress, comparison); break;
                case ValueType::Float  : Memory::UnknownSearch<float>   (startAddress, endAddress, comparison); break;
                case ValueType::Double : Memory::UnknownSearch<double>  (startAddress, endAddress, comparison); break;
                default: throw std::runtime_error("Unknown value type");
            }
        }
        catch(std::runtime_error& e)
        {
            Log(std::format("Error: {}", e.what()));
        }

        // プログレス通知解除
        Memory::SetProgressCallback(nullptr);

        // 成功通知
        Packet response(context);
        response.Write(Command::ID::UnknownSearch);
        response.Write(Command::Response::Success);
        response.Send();
    }
}