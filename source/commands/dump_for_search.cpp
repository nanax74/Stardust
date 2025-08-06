#include "commands/dump_for_search.hpp"
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
    void DumpForSearch::Process(Packet& packet)
    {
        auto context = packet.GetContext();

        // 進捗通知の設定
        Memory::SetProgressCallback([context](float progress)
        {
            Packet response(context);
            response.Write(ClientCommand::DumpForSearch);
            response.Write(Command::Response::Progress);
            response.Write(progress);
            response.Send();
        });

        // パラメータの読み取り
        uint32_t startAddress = packet.Read<uint32_t>();
        uint32_t endAddress = packet.Read<uint32_t>();
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>());

        try
        {
            switch(valueType)
            {
                case ValueType::Int8   : Memory::Dump<int8_t>  (startAddress, endAddress); break;
                case ValueType::UInt8  : Memory::Dump<uint8_t> (startAddress, endAddress); break;
                case ValueType::Int16  : Memory::Dump<int16_t> (startAddress, endAddress); break;
                case ValueType::UInt16 : Memory::Dump<uint16_t>(startAddress, endAddress); break;
                case ValueType::Int32  : Memory::Dump<int32_t> (startAddress, endAddress); break;
                case ValueType::UInt32 : Memory::Dump<uint32_t>(startAddress, endAddress); break;
                case ValueType::Int64  : Memory::Dump<int64_t> (startAddress, endAddress); break;
                case ValueType::UInt64 : Memory::Dump<uint64_t>(startAddress, endAddress); break;
                case ValueType::Float  : Memory::Dump<float>   (startAddress, endAddress); break;
                case ValueType::Double : Memory::Dump<double>  (startAddress, endAddress); break;
                default: throw std::runtime_error("Unsupported ValueType");
            }
        }
        catch(std::runtime_error& e)
        {
            Log(std::format("Error: {}", e.what()));
        }

        // メタデータ保存
        if(!Memory::Search_SaveMeta(startAddress, endAddress, valueType))
        {
            throw std::runtime_error("Failed to save search meta data");
        }

        Memory::SetProgressCallback(nullptr);

        // 成功通知
        Packet response(packet.GetContext());
        response.Write(ClientCommand::DumpForSearch);
        response.Write(Command::Response::Success);
        response.Send();
    }
}
