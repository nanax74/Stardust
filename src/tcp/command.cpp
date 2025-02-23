#include "command.hpp"

using namespace std;

#define COMMAND_READ_8 0x00
#define COMMAND_READ_16 0x01
#define COMMAND_READ_32 0x02

#define RESPONSE_VALUE 0x00

map<unsigned char, string> commandMap =
{
    {COMMAND_READ_8, "Read8"},
    {COMMAND_READ_16, "Read16"},
    {COMMAND_READ_32, "Read32"}
};

string GetCommandTypeStr(unsigned char command)
{
    auto it = commandMap.find(command);
    if(it != commandMap.end())
    {
        return it->second;
    }
    return "Unknown";
}

namespace TCPServer
{
    void ProcessCommand(unsigned char* buffer, int length)
    {
        if (length < 1) return; // コマンドが空なら無視

        unsigned char command = buffer[0];  // コマンドの種類

        buffer++;   // 1バイト進む
        length--;   // 長さを1減らす

        WHBLogPrintf("CommandType:%s", GetCommandTypeStr(command).c_str());  // コマンドの種類を出力

        // 受け取ったコマンドを文字列として出力
        ostringstream os;
        for (unsigned char* ptr = buffer; ptr < buffer + length; ptr++)
        {
            os << hex << setw(2) << setfill('0') << (int)(*ptr);
        }
        WHBLogPrintf("Command:%s", os.str().c_str());

        switch (command)
        {
            case COMMAND_READ_8:
            {
                if (length < 4) break; // アドレスが足りない場合は無視

                unsigned int address = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得
                unsigned char value = *(unsigned char*)address;  // 1byte読み取り

                unsigned char response[2] = {RESPONSE_VALUE};   // 値返しのヘッダ
                *(unsigned char*)(response + 1) = value;    // 値を返す

                SendData(response, 2);  // 結果を送信
                break;
            }
            case COMMAND_READ_16:
            {
                if (length < 4) break; // アドレスが足りない場合は無視

                unsigned int address = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得
                unsigned short value = *(unsigned short*)address;  // 2byte読み取り

                unsigned char response[3] = {RESPONSE_VALUE};   // 値返しのヘッダ
                *(unsigned short*)(response + 1) = value;    // 値を返す

                SendData(response, 3);  // 結果を送信
                break;
            }
            case COMMAND_READ_32:
            {
                if (length < 4) break; // アドレスが足りない場合は無視

                unsigned int address = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得
                unsigned int value = *(unsigned int*)address;  // 4byte読み取り

                unsigned char response[5] = {RESPONSE_VALUE};   // 値返しのヘッダ
                *(unsigned int*)(response + 1) = value;    // 値を返す

                SendData(response, 5);  // 結果を送信
                break;
            }
            default:
            {
                unsigned char response[1] = {0xFF}; // 未知のコマンド
                SendData(response, 1);
                break;
            }
        }
    }
}