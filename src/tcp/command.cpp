#include "command.hpp"

using namespace std;
using namespace memory;
using namespace data;

#define DATA_HEDDER 1   // ヘッダーの大きさ

#define COMMAND_READ 0x00
#define COMMAND_WRITE 0x01

#define RESPONSE_SUCCESS 0x00
#define RESPONSE_VALUE 0x01
#define RESPONSE_FAILED 0xFF

map<unsigned char, string> commandMap =
{
    {COMMAND_READ, "Read"},
    {COMMAND_WRITE, "Write"}
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
    void ResponseFailed(int socket)
    {
        bytes response;
        response.push_back(RESPONSE_FAILED);
        SendData(socket, response);
    }

    void ProcessCommand(int socket, bytes& data)
    {
        int dataSize = data.size();

        if(dataSize == 0) return;

        unsigned char command = data[0];  // コマンドの種類

        WHBLogPrintf("CommandType:%s", GetCommandTypeStr(command).c_str());  // コマンドの種類を出力

        // 受け取ったコマンドを文字列として出力
        ostringstream os;
        for (int i = 0; i < dataSize; i++)
        {
            os << hex << setw(2) << setfill('0') << (int)data[i];
        }
        WHBLogPrintf("Command:%s", os.str().c_str());

        switch (command)
        {
            case COMMAND_READ:
            {
                // 開始アドレス + 読み取る長さ (例:1234567800001234 -> 12345678から1234byte読み取られる)
                if (dataSize < DATA_HEDDER + (int)sizeof(unsigned int) * 2) ResponseFailed(socket); // バイト数が足りない場合は失敗応答を送信

                unsigned int startAddress = data.getUInt(DATA_HEDDER);  // 読み取るアドレスを取得
                unsigned int readSize = data.getUInt(DATA_HEDDER + (int)sizeof(unsigned int));

                bytes response;
                response.push_back(RESPONSE_VALUE);

                for(unsigned int i = 0; i < readSize; i++)
                {
                    unsigned int address = startAddress + i;
                    if(OSIsAddressValid(address))
                    {
                        unsigned char value = ReadUInt8(address);
                        response.append(value);
                    }
                    else
                    {
                        ResponseFailed(socket);
                        break;
                    }
                }
                SendData(socket, response);
                break;
            }
            case COMMAND_WRITE:
            {
                // 開始アドレス + 書き込む値 (例:123456780123456789ABCDEF -> 12345678に8byteの値0123456789ABCDEFが書き込まれる)
                if (dataSize < DATA_HEDDER + (int)sizeof(unsigned int) + (int)sizeof(unsigned char)) ResponseFailed(socket); // バイト数が足りない場合は失敗応答を送信

                unsigned int startAddress = data.getUInt(DATA_HEDDER);  // 書き込むメモリアドレスを取得
                bytes values;
                values.assign(data.begin() + DATA_HEDDER + (int)sizeof(unsigned int), data.end());  // 書き込む値を取得
                unsigned int writeSize = values.size();

                bytes response;
                response.push_back(RESPONSE_VALUE);

                for(unsigned int i = 0; i < writeSize; i++)
                {
                    unsigned int address = startAddress + i;
                    if(OSIsAddressValid(address))
                    {
                        unsigned char value = values[i];
                        WriteUInt8(address, value);
                    }
                    else
                    {
                        ResponseFailed(socket);
                        break;
                    }
                }
                SendData(socket, response);
                break;
            }
            default:
            {
                ResponseFailed(socket);
                break;
            }
        }
    }
}