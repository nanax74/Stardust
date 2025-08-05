#include "commands/read_multiple_memory.hpp"
#include "logger.hpp"

#include <vector>
#include <whb/log.h>
#include <coreinit/memorymap.h>

#include "commands/common.hpp"

namespace Command
{
    // 複数のメモリアドレスから値を読み取るコマンドハンドラ
    // ValueType, AddressCount, Addresses...
    void ReadMultipleMemory::Process(Packet& packet)
    {
        std::shared_ptr<SocketContext> context = packet.GetContext();
        
        ValueType valueType = static_cast<ValueType>(packet.Read<uint32_t>()); // 読み取るべき値の型を取得
        uint32_t addressesCount = packet.Read<uint32_t>(); // 読み取るべき値の数を取得

        // クライアントへ返信するレスポンスパケットを初期化
        Packet response(context);
        response.Write(Command::ID::ReadMultipleMemory); // コマンドID
        response.Write(Command::Response::Data);         // 応答タイプ:データ
        response.Write(valueType);                       // 値の型
        response.Write(addressesCount);                  // 値の数

        for (uint32_t i = 0; i < addressesCount; ++i)
        {
            uint32_t address = packet.Read<uint32_t>(); // アドレス
            response.Write(address); // アドレスをパケットに書き込み
            ReadMemoryAndWritePacketValue(response, valueType, address); // アドレスと読み取った値をパケットに書き込み
        }
        response.Send(); // 構築されたレスポンスパケットをクライアントに送信
        Log::Printf("ReadMultipleMemory command finished.");
    }

    // アドレスの安全確認あり
    bool ReadMemoryAndWritePacketValue(Packet& packet, ValueType valueType, uint32_t address)
    {
        if(OSIsAddressValid(address))
        {
            switch(valueType)
            {
                case ValueType::Int8:
                {
                    int8_t value = *reinterpret_cast<int8_t*>(address);
                    packet.Write(value);
                }
                case ValueType::UInt8:
                {
                    uint8_t value = *reinterpret_cast<uint8_t*>(address);
                    packet.Write(value);
                }
                case ValueType::Int16:
                {
                    int16_t value = *reinterpret_cast<int16_t*>(address);
                    packet.Write(value);
                }
                case ValueType::UInt16:
                {
                    uint16_t value = *reinterpret_cast<uint16_t*>(address);
                    packet.Write(value);
                }
                case ValueType::Int32:
                {
                    int32_t value = *reinterpret_cast<int32_t*>(address);
                    packet.Write(value);
                }
                case ValueType::UInt32:
                {
                    uint32_t value = *reinterpret_cast<uint32_t*>(address);
                    packet.Write(value);
                }
                case ValueType::Int64:
                {
                    int64_t value = *reinterpret_cast<int64_t*>(address);
                    packet.Write(value);
                }
                case ValueType::UInt64:
                {
                    uint64_t value = *reinterpret_cast<uint64_t*>(address);
                    packet.Write(value);
                }
                case ValueType::Float:
                {
                    float value = *reinterpret_cast<float*>(address);
                    packet.Write(value);
                }
                case ValueType::Double:
                {
                    double value = *reinterpret_cast<double*>(address);
                    packet.Write(value);
                }
                default:
                {
                    return false;
                }
            }
            return true;
        }
        else
        {
            switch(valueType)
            {
                case ValueType::Int8:
                {
                    int8_t value = 0;
                    packet.Write(value);
                }
                case ValueType::UInt8:
                {
                    uint8_t value = 0;
                    packet.Write(value);
                }
                case ValueType::Int16:
                {
                    int16_t value = 0;
                    packet.Write(value);
                }
                case ValueType::UInt16:
                {
                    uint16_t value = 0;
                    packet.Write(value);
                }
                case ValueType::Int32:
                {
                    int32_t value = 0;
                    packet.Write(value);
                }
                case ValueType::UInt32:
                {
                    uint32_t value = 0;
                    packet.Write(value);
                }
                case ValueType::Int64:
                {
                    int64_t value = 0;
                    packet.Write(value);
                }
                case ValueType::UInt64:
                {
                    uint64_t value = 0;
                    packet.Write(value);
                }
                case ValueType::Float:
                {
                    float value = 0.0f;
                    packet.Write(value);
                }
                case ValueType::Double:
                {
                    double value = 0.0;
                    packet.Write(value);
                }
                default:
                {
                    return false;
                }
            }
            return false;
        }
    }
}