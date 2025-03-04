#include "command.hpp"

// std
#include <map>
#include <filesystem>

// wut
#include <coreinit/memorymap.h>
#include <whb/log.h>

// stardust
#include "data.hpp"
#include "../utils/compare.hpp"
#include "../utils/math.hpp"
#include "../utils/memory.hpp"

#define COMMAND_READ 0x00 // バイト数制限付き
#define COMMAND_WRITE 0x01 // バイト数制限付き
#define COMMAND_SEARCH_SPEC 0x02 // 結果が1MBを超えた場合、結果を送信せずにサイズだけ送信、結果はファイルに保存
#define COMMAND_SEARCH_DUMP 0x03 // サーチ用のダンプ
#define COMMAND_SEARCH_UNKN 0x04 // ダンプからサーチ

#define VALUE_CHAR 0x00
#define VALUE_UCHAR 0x01
#define VALUE_SHORT 0x02
#define VALUE_USHORT 0x03
#define VALUE_INT 0x04
#define VALUE_UINT 0x05
#define VALUE_LONG 0x06
#define VALUE_ULONG 0x07

using namespace std;
using namespace memory;
using namespace data;

// コマンドの種類
map<unsigned char, string> commandMap =
{
    {COMMAND_READ, "Read"},
    {COMMAND_WRITE, "Write"},
    {COMMAND_SEARCH_SPEC, "Specific Search"},
    {COMMAND_SEARCH_DUMP, "Dump"},
    {COMMAND_SEARCH_UNKN, "Unknown Search"}
};

// コマンドの種類を文字列で取得
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
    void RepsonseFailed(int socket)
    {
        bytes response;
        response.append(RESPONSE_FAILED); // 失敗応答
        SendPacket(socket, response, DATA_TYPE_RESPONSE);
    }

    // コマンドを処理
    void ProcessCommand(int socket, bytes& data)
    {
        bytes response;

        unsigned int dataLength = data.size();

        if(dataLength == 0)  // コマンドのサイズが0なら中止
        {
            RepsonseFailed(socket);
            return;
        };

        unsigned char command = data[0];  // コマンドの種類

        string commandStr = GetCommandTypeStr(command); // コマンドの種類を文字列に変換
        WHBLogPrintf("CommandType:%s", commandStr.c_str());  // コマンドの種類を出力

        // 受け取ったコマンドを文字列として出力
        ostringstream os;
        for(unsigned int i = 0; i < dataLength; i++) // コマンドの長さの分だけ繰り返す
        {
            os << hex << setw(2) << setfill('0') << (int)data[i]; // 文字列ストリームにバイトを追加
        }
        WHBLogPrintf("Command:%s", os.str().c_str()); // 出力

        unsigned int dataIndex = HEADER_SIZE;

        auto startTime = chrono::high_resolution_clock::now(); // 計測開始

        switch (command) // コマンドの種類に応じて処理
        {
            case COMMAND_READ: // データ読み取り
            {
                // 開始アドレス + 読み取る長さ (例:10000000 00001000 -> 0x10000000から0x1000byte読み取られる)
                
                if(dataLength < sizeof(unsigned int) * 2) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                unsigned int startAddress = data.get<unsigned int>(dataIndex);  // 読み取るアドレスを取得
                dataIndex += sizeof(startAddress);
                unsigned int readSize = data.get<unsigned int>(dataIndex); // 読み取るサイズを取得

                if(readSize > freeMemorySize) // 読み取るデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }

                for(unsigned int i = 0; i < readSize; i++) // すべてのデータを読み取るまでループ
                {
                    unsigned int address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        unsigned char value = base::ReadMemory<unsigned char>(address); // 1byteずつ読み取り
                        response.append(value); // 応答に追加
                    }
                    else // 不正なアドレス
                    {
                        WHBLogPrintf("%s failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                        RepsonseFailed(socket);
                        break;
                    }
                }
                WHBLogPrintf("%s is Success!", commandStr.c_str()); // 成功！
                break; // 終了
            }
            case COMMAND_WRITE:
            {
                // 開始アドレス + 書き込む値 (例:10000000 3F80000040000000 -> 0x10000000に8byteの値0x3F80000040000000が書き込まれる)

                if(dataLength < HEADER_SIZE + sizeof(unsigned int) + sizeof(unsigned char)) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                unsigned int startAddress = data.get<unsigned int>(dataIndex); // 書き込むメモリアドレスを取得
                dataIndex += sizeof(startAddress);
                bytes values; // 書き込む値のバッファ
                values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                unsigned int writeSize = values.size(); // 書き込むデータの長さ

                if(writeSize > freeMemorySize) // 書き込むデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }

                bytes response; // 応答
                response.push_back(RESPONSE_SUCCESS);

                for(unsigned int i = 0; i < writeSize; i++) // すべてのデータを書き込むまでループ
                {
                    unsigned int address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        unsigned char value = values[i]; // 書き込む値
                        base::WriteMemory<unsigned char>(address, value); // アドレスに値を書き込む
                    }
                    else // 不正なアドレス
                    {
                        WHBLogPrintf("%s is Failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                        RepsonseFailed(socket);
                        break;
                    }
                }
                break;
            }
            case COMMAND_SEARCH_SPEC:
            {
                /*
                // 開始アドレス + サーチする長さ + サーチする値 (例:10000000 00001000 3F800000 -> 10000000から1000byteの範囲で値が3F800000のアドレスが検索される)
                if(dataSize < DATA_HEDDER + (int)sizeof(unsigned int) * 3) break;

                unsigned int startAddress = data.get<unsigned int>(DATA_HEDDER);  // 書き込むメモリアドレスを取得
                unsigned int searchSize = data.get<unsigned int>(DATA_HEDDER + (int)sizeof(startAddress));
                unsigned int searchValue = data.get<unsigned int>(DATA_HEDDER + (int)sizeof(startAddress) + (int)sizeof(searchSize));

                bytes response;
            
                for(unsigned int i = 0; i < searchSize / sizeof(unsigned int); i++)
                {
                    unsigned int address = startAddress + i * sizeof(unsigned int);
                    if(OSIsAddressValid(address))
                    {
                        unsigned int value = ReadMemory<unsigned int>(address);
                        if(value == searchValue)
                        {
                            response.append(address);
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                SendData(socket, response);
                */
                break;
            }
            case COMMAND_SEARCH_DUMP:
            {
                // 開始アドレス + ダンプする長さ + 値の型 (例:10000000 00001000 05 -> 0x10000000からunsigned intで0x1000byte読み取りダンプする)

                // コマンドから情報を取得
                unsigned int address = data.get<unsigned int>(dataIndex);
                dataIndex += sizeof(address);
                unsigned int size = data.get<unsigned int>(dataIndex);
                dataIndex += sizeof(size);
                unsigned char valueType = data.get<unsigned char>(dataIndex);

                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";

                switch (valueType)
                {
                    case VALUE_CHAR:
                    {
                        advanced::DumpMemory<char>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UCHAR:
                    {
                        advanced::DumpMemory<unsigned char>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_SHORT:
                    {
                        advanced::DumpMemory<short>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_USHORT:
                    {
                        advanced::DumpMemory<unsigned short>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_INT:
                    {
                        advanced::DumpMemory<int>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT:
                    {
                        advanced::DumpMemory<unsigned int>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_LONG:
                    {
                        advanced::DumpMemory<long>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_ULONG:
                    {
                        advanced::DumpMemory<unsigned long>(socket, dirPath, "Search", address, size);
                        break;
                    }
                }
                break;
            }
            case COMMAND_SEARCH_UNKN:
            {
                // 開始アドレス(例:05 -> 前回のサーチ結果のサイズ分のメモリアドレスをunsigned intでサーチ)

                // コマンドから情報を取得
                unsigned char valueType = data.get<unsigned char>(dataIndex);
            
                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";
            
                switch (valueType)
                {
                    case VALUE_CHAR:
                    {
                        MemoryList<char> src;
                        advanced::SearchMemory<char>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_UCHAR:
                    {
                        MemoryList<unsigned char> src;
                        advanced::SearchMemory<unsigned char>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_SHORT:
                    {
                        MemoryList<short> src;
                        advanced::SearchMemory<short>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_USHORT:
                    {
                        MemoryList<unsigned short> src;
                        advanced::SearchMemory<unsigned short>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_INT:
                    {
                        MemoryList<int> src;
                        advanced::SearchMemory<int>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_UINT:
                    {
                        MemoryList<unsigned int> src;
                        advanced::SearchMemory<unsigned int>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_LONG:
                    {
                        MemoryList<long> src;
                        advanced::SearchMemory<long>(socket, dirPath, "Search");
                        break;
                    }
                    case VALUE_ULONG:
                    {
                        MemoryList<unsigned long> src;
                        advanced::SearchMemory<unsigned long>(socket, dirPath, "Search");
                        break;
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
        SendPacket(socket, response, DATA_TYPE_RESPONSE);
        auto endTime = chrono::high_resolution_clock::now(); // 計測終了
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count(); // 経過時間を算出
        WHBLogPrintf("Processing complete: %lld ms", duration); // かかった時間を表示
    }
}