#pragma once

// std
#include <string>

// stardust
#include "../utils/bytes.hpp"
#include "../utils/memory.hpp"

#define HEADER_SIZE 1

const uint8_t RESPONSE_SUCCESS = 0x00; // 成功応答
const uint8_t RESPONSE_VALUE   = 0x01; // 値応答
const uint8_t RESPONSE_ERROR  = 0xFF; // 失敗応答

namespace TCPServer
{
    void ProcessCommand(int32_t socket, Bytes& data);

    void SendSuccess(int socket, auto startTime, Bytes response);
    void SendError(int socket, auto startTime);
}