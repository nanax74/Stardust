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

const uint8_t ERROR_COMMAND_INVALID = 0x00; // 無効なアドレス
const uint8_t ERROR_ADDRESS_INVALID = 0x01; // 無効なコマンド
const uint8_t ERROR_DATA_TOO_LONG = 0x02; // 長すぎるデータ
const uint8_t ERROR_DATA_EMPTY = 0x03; // エラーなし
const uint8_t ERROR_VALUE_TYPE_INVALID = 0x04; // 無効な値の型

namespace TCPServer
{
    void ProcessCommand(int32_t socket, Bytes& data);

    void SendSuccess(int socket, auto startTime, Bytes response);
    void SendError(int socket, auto startTime, uint8_t errorCode);
}