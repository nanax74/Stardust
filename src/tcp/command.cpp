#include "command.hpp"

using namespace std;

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
    void ResponseFailed()
    {
        unsigned char response[1] = {RESPONSE_FAILED};
        SendData(response, 1);
    }

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

                if(OSIsAddressValid(address))
                {
                    unsigned char value = *(unsigned char*)address;
                    unsigned char response[2] = {RESPONSE_VALUE};
                    *(unsigned char*)(response + 1) = value;
                    SendData(response, 2);
                }
                else
                {
                    ResponseFailed();
                }

                break;
            }
            case COMMAND_READ_16:
            {
                if (length < 4) break; // アドレスが足りない場合は無視

                unsigned int address = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得

                if(OSIsAddressValid(address))
                {
                    unsigned short value = *(unsigned short*)address;
                    unsigned char response[3] = {RESPONSE_VALUE};
                    *(unsigned short*)(response + 1) = value;
                    SendData(response, 3);
                }
                else
                {
                    ResponseFailed();
                }

                break;
            }
            case COMMAND_READ_32:
            {
                if (length < 4) break; // アドレスが足りない場合は無視

                unsigned int address = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得

                if(OSIsAddressValid(address))
                {
                    unsigned int value = *(unsigned int*)address;
                    unsigned char response[5] = {RESPONSE_VALUE};
                    *(unsigned int*)(response + 1) = value;
                    SendData(response, 5);
                }
                else
                {
                    ResponseFailed();
                }

                break;
            }
            case COMMAND_READ_ANY:
            {
                if (length < 8) break; // アドレスが足りない場合は無視

                unsigned int startAddress = *(unsigned int*)buffer;  // バッファから読み取るアドレスを取得
                unsigned int endAddress = *(unsigned int*)(buffer + 4);  // バッファから読み取るアドレスを取得
                int readSize = endAddress - startAddress;

                unsigned char response[readSize];

                for(int i = 0; i < readSize; i++)
                {
                    if(OSIsAddressValid(startAddress + i))
                    {
                        unsigned char value = *(unsigned char*)(startAddress + i);
                        
                        *(unsigned char*)(response + 1 + i) = value;
                    }
                    else
                    {
                        ResponseFailed();
                        break;
                    }
                }
                response[0] = RESPONSE_VALUE;
                SendData(response, readSize);
                break;
            }
            default:
            {
                ResponseFailed();
                break;
            }
        }
    }
}