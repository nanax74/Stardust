#pragma once

// std
#include <map>
#include <string>

// stardust
#include "../utils/bytes.hpp"
#include "../utils/memory.hpp"

#define HEADER_SIZE 1

namespace Command
{
    const uint8_t RESPONSE_SUCCESS = 0x00; // 成功応答
    const uint8_t RESPONSE_VALUE   = 0x01; // 値応答
    const uint8_t RESPONSE_ERROR  = 0xFF; // 失敗応答

    const uint8_t ERROR_COMMAND_INVALID = 0x00; // 無効なアドレス
    const uint8_t ERROR_ADDRESS_INVALID = 0x01; // 無効なコマンド
    const uint8_t ERROR_DATA_TOO_LONG = 0x02; // 長すぎるデータ
    const uint8_t ERROR_DATA_EMPTY = 0x03; // エラーなし
    const uint8_t ERROR_VALUE_TYPE_INVALID = 0x04; // 無効な値の型

    const uint8_t COMMAND_READ                = 0x00; // バイト数制限付き
    const uint8_t COMMAND_WRITE               = 0x01; // バイト数制限付き
    const uint8_t COMMAND_SEARCH_DUMP         = 0x02; // サーチ用のダンプ
    const uint8_t COMMAND_SEARCH_SPEC         = 0x03; // ダンプからサーチ
    const uint8_t COMMAND_SEARCH_UNKN         = 0x04; // ダンプからサーチ
    const uint8_t COMMAND_WRITE_KERNEL        = 0x05; // カーネル書き込み
    const uint8_t COMMAND_SET_DATA_BREAKPOINT = 0x06; // データブレークポイント設定
    const uint8_t COMMAND_SEND_CONTEXT        = 0x07; // コンテキスト送信

    const uint8_t VALUE_INT8   = 0x00;
    const uint8_t VALUE_UINT8  = 0x01;
    const uint8_t VALUE_INT16  = 0x02;
    const uint8_t VALUE_UINT16 = 0x03;
    const uint8_t VALUE_INT32  = 0x04;
    const uint8_t VALUE_UINT32 = 0x05;
    const uint8_t VALUE_INT64  = 0x06;
    const uint8_t VALUE_UINT64 = 0x07;
    const uint8_t VALUE_FLOAT  = 0x08;
    const uint8_t VALUE_DOUBLE = 0x09;

    // コマンドの種類
    extern const std::map<uint8_t, std::string> commandNameMap;

    std::string GetCommandTypeStr(uint8_t command);
    void PrintCommandLog(uint32_t length, Bytes& data);

    // コマンドを処理
    void ProcessCommand(int32_t socket, Bytes& data);

    void SendSuccess(int socket, auto startTime, Bytes response);
    void SendError(int socket, auto startTime, uint8_t errorCode);

    void SetContextPacket(Bytes& response);
};