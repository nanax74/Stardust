#pragma once

// std
#include <string>

// stardust
#include "../utils/bytes.hpp"

class Packet
{
private:
    const uint32_t MAX_PACKET_SIZE = 1024 * 1024; // パケットの最大サイズ

public:
    static const uint8_t DATA_TYPE_NULL = 0x00;
    static const uint8_t DATA_TYPE_DATA  = 0x01;
    static const uint8_t DATA_TYPE_PROGRESS = 0x02;

public:
    Bytes Data;
    uint32_t Type;
    Packet();
    Packet(Bytes& data, uint32_t type);

public:
    int32_t SendPacket(int32_t socket);
    int32_t ReceivePacket(int32_t socket);
};