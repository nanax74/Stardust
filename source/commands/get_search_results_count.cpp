#include "commands/get_search_results_count.hpp"
#include "commands/common.hpp"

namespace Command
{
    void GetSearchResultsCount::Process(Packet& packet)
    {
        auto context = packet.GetContext();

        // パラメータの読み取り
        uint32_t startAddress, endAddress;
        ValueType valueType;
        if(!Memory::Search_LoadMeta(startAddress, endAddress, valueType))
        {
            throw "Failed to load search meta data";
        }

        uint32_t count = 0;

        try
        {
            switch (valueType)
            {
                case ValueType::Int8   : count = Memory::GetSearchResultsCount<int8_t>  (); break;
                case ValueType::UInt8  : count = Memory::GetSearchResultsCount<uint8_t> (); break;
                case ValueType::Int16  : count = Memory::GetSearchResultsCount<int16_t> (); break;
                case ValueType::UInt16 : count = Memory::GetSearchResultsCount<uint16_t>(); break;
                case ValueType::Int32  : count = Memory::GetSearchResultsCount<int32_t> (); break;
                case ValueType::UInt32 : count = Memory::GetSearchResultsCount<uint32_t>(); break;
                case ValueType::Int64  : count = Memory::GetSearchResultsCount<int64_t> (); break;
                case ValueType::UInt64 : count = Memory::GetSearchResultsCount<uint64_t>(); break;
                case ValueType::Float  : count = Memory::GetSearchResultsCount<float>   (); break;
                case ValueType::Double : count = Memory::GetSearchResultsCount<double>  (); break;
                default: throw std::runtime_error("Unsupported ValueType for SpecificSearch");
            }

            Packet packet(context);
            packet.Write(Command::ID::GetSearchResultsCount);
            packet.Write(Command::Response::Success);
            packet.Write(count);
            packet.Send();
        }
        catch(std::runtime_error& e)
        {
            Log("Failed to get current search results");

            Packet packet(context);
            packet.Write(Command::ID::GetSearchResultsCount);
            packet.Write(Command::Response::Failure);
            packet.Send();
        }
    }
}

