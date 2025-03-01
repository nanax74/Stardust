#pragma once

#include "../utils/bytes.hpp"

#define DATA_HEDDER 1   // ヘッダーの大きさ

#define COMMAND_READ 0x00 // バイト数制限付き
#define COMMAND_WRITE 0x01 // バイト数制限付き
#define COMMAND_DUMP 0x02 // バイト数無制限、読み取ったデータは"fs:/vol/external01/wiiu/Stardust/Dump.bin"に保存
#define COMMAND_SEARCH_SPEC 0x03 // 結果が1MBを超えた場合、結果を送信せずにサイズだけ送信、結果はファイルに保存
#define COMMAND_SEARCH_UNKN 0x04

#define RESPONSE_SUCCESS 0x00 // 成功応答
#define RESPONSE_VALUE 0x01 // 値応答
#define RESPONSE_FAILED 0xFF // 失敗応答

#define FILE_DUMP 0x00
#define FILE_SEARCH 0x01

namespace TCPServer
{
    void ProcessCommand(int socket, bytes& data);
}