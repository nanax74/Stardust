#include "commands/unknown_search.hpp"
#include "commands/common.hpp"

#include <format>

namespace Command
{
    void UnknownSearch(Packet& packet)
    {
        auto context = packet.GetContext();

        // プログレス通知の設定
        Memory::SetProgressCallback([context](float progress)
        {
            Packet response(context);
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

        auto sendResult = [context](uint32_t address, uint32_t value, Command::ID cmd)
        {
            Packet response(context);
            response.Write(std::to_underlying(cmd));
            response.Write(std::to_underlying(Command::Response::Data));
            response.Write(address);
            response.Write(value);
            response.Send();
        };

        auto callbackInt8 = Memory::MakeSendSearchResultCallback<int8_t>(context);
        auto callbackUInt8 = Memory::MakeSendSearchResultCallback<uint8_t>(context);
        auto callbackInt16 = Memory::MakeSendSearchResultCallback<int16_t>(context);
        auto callbackUInt16 = Memory::MakeSendSearchResultCallback<uint16_t>(context);
        auto callbackInt32 = Memory::MakeSendSearchResultCallback<int32_t>(context);
        auto callbackUInt32 = Memory::MakeSendSearchResultCallback<uint32_t>(context);
        auto callbackInt64 = Memory::MakeSendSearchResultCallback<int64_t>(context);
        auto callbackUInt64 = Memory::MakeSendSearchResultCallback<uint64_t>(context);
        auto callbackFloat = Memory::MakeSendSearchResultCallback<float>(context);
        auto callbackDouble = Memory::MakeSendSearchResultCallback<double>(context);

        switch (valueType)
        {
            case ValueType::Int8: Memory::UnknownSearch<int8_t>(startAddress, endAddress, valueType, comparison, callbackInt8); break;
            case ValueType::UInt8: Memory::UnknownSearch<uint8_t>(startAddress, endAddress, valueType, comparison, callbackUInt8); break;
            case ValueType::Int16: Memory::UnknownSearch<int16_t>(startAddress, endAddress, valueType, comparison, callbackInt16); break;
            case ValueType::UInt16: Memory::UnknownSearch<uint16_t>(startAddress, endAddress, valueType, comparison, callbackUInt16); break;
            case ValueType::Int32: Memory::UnknownSearch<int32_t>(startAddress, endAddress, valueType, comparison, callbackInt32); break;
            case ValueType::UInt32: Memory::UnknownSearch<uint32_t>(startAddress, endAddress, valueType, comparison, callbackUInt32); break;
            case ValueType::Int64: Memory::UnknownSearch<int64_t>(startAddress, endAddress, valueType, comparison, callbackInt64); break;
            case ValueType::UInt64: Memory::UnknownSearch<uint64_t>(startAddress, endAddress, valueType, comparison, callbackUInt64); break;
            case ValueType::Float: Memory::UnknownSearch<float>(startAddress, endAddress, valueType, comparison, callbackFloat); break;
            case ValueType::Double: Memory::UnknownSearch<double>(startAddress, endAddress, valueType, comparison, callbackDouble); break;
            default: throw std::runtime_error("Unknown value type");
        }

        // プログレス通知解除
        Memory::SetProgressCallback(nullptr);
    }
}