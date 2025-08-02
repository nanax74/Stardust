#include "commands/get_search_results.hpp"
#include "commands/common.hpp"

namespace Command
{
    void GetSearchResults::Process(Packet& packet)
    {
        auto context = packet.GetContext();

        auto callbackInt8   = Memory::MakeSendSearchResultCallback<int8_t>  (context);
        auto callbackUInt8  = Memory::MakeSendSearchResultCallback<uint8_t> (context);
        auto callbackInt16  = Memory::MakeSendSearchResultCallback<int16_t> (context);
        auto callbackUInt16 = Memory::MakeSendSearchResultCallback<uint16_t>(context);
        auto callbackInt32  = Memory::MakeSendSearchResultCallback<int32_t> (context);
        auto callbackUInt32 = Memory::MakeSendSearchResultCallback<uint32_t>(context);
        auto callbackInt64  = Memory::MakeSendSearchResultCallback<int64_t> (context);
        auto callbackUInt64 = Memory::MakeSendSearchResultCallback<uint64_t>(context);
        auto callbackFloat  = Memory::MakeSendSearchResultCallback<float>   (context);
        auto callbackDouble = Memory::MakeSendSearchResultCallback<double>  (context);

        // パラメータの読み取り
        uint32_t startAddress, endAddress;
        ValueType valueType;
        if(!Memory::Search_LoadMeta(startAddress, endAddress, valueType))
        {
            throw "Failed to load search meta data";
        }

        try
        {
            switch (valueType)
            {
                case ValueType::Int8   : Memory::GetSearchResults<int8_t>  (startAddress, endAddress, callbackInt8  ); break;
                case ValueType::UInt8  : Memory::GetSearchResults<uint8_t> (startAddress, endAddress, callbackUInt8 ); break;
                case ValueType::Int16  : Memory::GetSearchResults<int16_t> (startAddress, endAddress, callbackInt16 ); break;
                case ValueType::UInt16 : Memory::GetSearchResults<uint16_t>(startAddress, endAddress, callbackUInt16); break;
                case ValueType::Int32  : Memory::GetSearchResults<int32_t> (startAddress, endAddress, callbackInt32 ); break;
                case ValueType::UInt32 : Memory::GetSearchResults<uint32_t>(startAddress, endAddress, callbackUInt32); break;
                case ValueType::Int64  : Memory::GetSearchResults<int64_t> (startAddress, endAddress, callbackInt64 ); break;
                case ValueType::UInt64 : Memory::GetSearchResults<uint64_t>(startAddress, endAddress, callbackUInt64); break;
                case ValueType::Float  : Memory::GetSearchResults<float>   (startAddress, endAddress, callbackFloat ); break;
                case ValueType::Double : Memory::GetSearchResults<double>  (startAddress, endAddress, callbackDouble); break;
                default: throw std::runtime_error("Unsupported ValueType for SpecificSearch");
            }

            Packet packet(context);
            packet.Write(Command::ID::GetSearchResults);
            packet.Write(Command::Response::Success);
            packet.Send();
        }
        catch(std::runtime_error& e)
        {
            Log("Failed to get current search results");

            Packet packet(context);
            packet.Write(Command::ID::GetSearchResults);
            packet.Write(Command::Response::Failure);
            packet.Send();
        }
    }
}

