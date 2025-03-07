#pragma once

// std
#include <string>

// stardust
#include "../utils/bytes.hpp"
#include "../utils/memory.hpp"

#define HEADER_SIZE 1

#define RESPONSE_SUCCESS 0x00 // 成功応答
#define RESPONSE_VALUE 0x01 // 値応答
#define RESPONSE_FAILED 0xFF // 失敗応答

namespace TCPServer
{
    void ProcessCommand(int32_t socket, bytes& data);
}