#include "packet.hpp"

// std
#include <array>
#include <bit>
#include <cstdint>
#include <vector>

// wut
#include "whb/log.h"

// stardust
#include "transfer.hpp"
#include "../utils/value.hpp"

using namespace std;

Packet::Packet()
{
    Data = Bytes();
    Type = 0;
}

Packet::Packet(Bytes& data, uint32_t type)
{
    Data = data;
    Type = type;
}

int32_t Packet::SendPacket(int32_t socket)
{
    uint32_t size = Data.size();
    WHBLogPrintf("Data size:%d", size);
    if(size == 0) // データが空
    {
        WHBLogPrintf("Data to send is empty");
        return -1;
    }
    else if(size > MAX_PACKET_SIZE) // データが大きすぎる
    {
        WHBLogPrintf("Data to send is too large:%dbytes", size);
        return -1;
    }

    Bytes header = Bytes();
    header.append(size);
    header.append(Type);
    int32_t result = Transfer::SendAll(socket, header); // ヘッダを送信
    if(result <= 0) return result; // 送信失敗
    return Transfer::SendAll(socket, Data); // データを送信
}

int32_t Packet::ReceivePacket(int32_t socket)
{
    size_t headerSize = sizeof(uint32_t) * 2; // データのサイズ
    Bytes header = Bytes(headerSize); // ヘッダのバッファ
    int32_t result = Transfer::ReceiveAll(socket, header);
    if(result <= 0) return result; // 受信失敗
    uint32_t size = std::bit_cast<int32_t>(array<uint8_t, 4>{header[0], header[1], header[2], header[3]});
    Type = std::bit_cast<int32_t>(array<uint8_t, 4>{header[4], header[5], header[6], header[7]});
    if(size > MAX_PACKET_SIZE)
    {
        WHBLogPrintf("Data too large to receive:%dbytes", size);
        return -1; // データが大きすぎる
    }
    if(size == 0) return -1;
    Data = Bytes(size);
    return Transfer::ReceiveAll(socket, Data);
}