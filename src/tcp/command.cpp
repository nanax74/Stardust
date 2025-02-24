#include "command.hpp"

using namespace std;
using namespace memory;

#define DATA_HEDDER 1   // ヘッダーの大きさ

#define COMMAND_READ_8 0x00
#define COMMAND_READ_16 0x01
#define COMMAND_READ_32 0x02
#define COMMAND_READ_ANY 0x03

#define RESPONSE_VALUE 0x00
#define RESPONSE_FAILED 0xFF

map<unsigned char, string> commandMap =
{
    {COMMAND_READ_8, "Read8"},
    {COMMAND_READ_16, "Read16"},
    {COMMAND_READ_32, "Read32"},
    {COMMAND_READ_ANY, "ReadAny"}
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
            case COMMAND_READ_8:
            {
                if (dataSize < DATA_HEDDER + 1) break; // アドレスが足りない場合は無視

                unsigned int address = data.getUInt(DATA_HEDDER);  // バッファから読み取るアドレスを取得

                if(OSIsAddressValid(address))
                {
                    unsigned char value = ReadUInt8(address);
                    bytes response;
                    response.push_back(RESPONSE_VALUE);
                    response.append(value);
                    SendData(socket, response);
                }
                else
                {
                    ResponseFailed(socket);
                }

                break;
            }
            case COMMAND_READ_16:
            {
                if (dataSize < DATA_HEDDER + 2) break; // アドレスが足りない場合は無視

                unsigned int address = data.getUInt(DATA_HEDDER);  // バッファから読み取るアドレスを取得

                if(OSIsAddressValid(address))
                {
                    unsigned short value = ReadUInt16(address);
                    bytes response;
                    response.push_back(RESPONSE_VALUE);
                    response.append(value);
                    SendData(socket, response);
                }
                else
                {
                    ResponseFailed(socket);
                }

                break;
            }
            case COMMAND_READ_32:
            {
                if (dataSize < DATA_HEDDER + 4) break; // アドレスが足りない場合は無視

                unsigned int address = data.getUInt(DATA_HEDDER);  // バッファから読み取るアドレスを取得

                if(OSIsAddressValid(address))
                {
                    unsigned short value = ReadUInt32(address);
                    bytes response;
                    response.push_back(RESPONSE_VALUE);
                    response.append(value);
                    SendData(socket, response);
                }
                else
                {
                    ResponseFailed(socket);
                }

                break;
            }
            case COMMAND_READ_ANY:
            {
                if (dataSize < DATA_HEDDER + 8) break; // アドレスが足りない場合は無視

                unsigned int startAddress = data.getUInt(DATA_HEDDER);  // バッファから読み取るアドレスを取得
                unsigned int endAddress = data.getUInt(DATA_HEDDER + 4);  // バッファから読み取るアドレスを取得

                if (startAddress >= endAddress)
                {
                    ResponseFailed(socket);
                    break;
                }

                int readSize = endAddress - startAddress;

                bytes response;
                response.push_back(RESPONSE_VALUE);

                for(int i = 0; i < readSize; i++)
                {
                    if(OSIsAddressValid(startAddress + i))
                    {
                        unsigned char value = ReadUInt8(startAddress + i);
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
            default:
            {
                ResponseFailed(socket);
                break;
            }
        }
    }
}