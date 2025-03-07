#include "command.hpp"

// std
#include <cstdint>
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
#define COMMAND_SEARCH_SPEC 0x02 // Todo
#define COMMAND_SEARCH_DUMP 0x03 // サーチ用のダンプ
#define COMMAND_SEARCH_UNKN 0x04 // ダンプからサーチ

#define VALUE_CHAR 0x00
#define VALUE_UCHAR 0x01
#define VALUE_SHORT 0x02
#define VALUE_USHORT 0x03
#define VALUE_INT 0x04
#define VALUE_UINT 0x05
#define VALUE_FLOAT 0x06
#define VALUE_DOUBLE 0x07

using namespace std;
using namespace memory;
using namespace data;

// コマンドの種類
map<uint8_t, string> commandMap =
{
    {COMMAND_READ, "Read"},
    {COMMAND_WRITE, "Write"},
    {COMMAND_SEARCH_SPEC, "Specific Search"},
    {COMMAND_SEARCH_DUMP, "Dump"},
    {COMMAND_SEARCH_UNKN, "Unknown Search"}
};

// コマンドの種類を文字列で取得
string GetCommandTypeStr(uint8_t command)
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
    void RepsonseFailed(int32_t socket)
    {
        bytes response;
        response.append(RESPONSE_FAILED); // 失敗応答
        SendPacket(socket, response, DATA_TYPE_RESPONSE);
    }

    // コマンドを処理
    void ProcessCommand(int32_t socket, bytes& data)
    {
        bytes response;

        uint32_t dataLength = data.size();

        if(dataLength == 0)  // コマンドのサイズが0なら中止
        {
            RepsonseFailed(socket);
            return;
        };

        uint8_t command = data[0];  // コマンドの種類

        string commandStr = GetCommandTypeStr(command); // コマンドの種類を文字列に変換
        WHBLogPrintf("CommandType:%s", commandStr.c_str());  // コマンドの種類を出力

        // 受け取ったコマンドを文字列として出力
        ostringstream os;
        for(uint32_t i = 0; i < dataLength; i++) // コマンドの長さの分だけ繰り返す
        {
            os << hex << setw(2) << setfill('0') << (int32_t)data[i]; // 文字列ストリームにバイトを追加
        }
        WHBLogPrintf("Command:%s", os.str().c_str()); // 出力

        uint32_t dataIndex = HEADER_SIZE;

        auto startTime = chrono::high_resolution_clock::now(); // 計測開始

        switch (command) // コマンドの種類に応じて処理
        {
            case COMMAND_READ: // データ読み取り
            {
                // 開始アドレス + 読み取る長さ (例:10000000 00001000 -> 0x10000000から0x1000byte読み取られる)
                
                if(dataLength < sizeof(uint32_t) * 2) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                uint32_t startAddress = data.get<uint32_t>(dataIndex);  // 読み取るアドレスを取得
                dataIndex += sizeof(startAddress);
                uint32_t readSize = data.get<uint32_t>(dataIndex); // 読み取るサイズを取得

                if(readSize > freeMemorySize) // 読み取るデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }

                for(uint32_t i = 0; i < readSize; i++) // すべてのデータを読み取るまでループ
                {
                    uint32_t address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        uint8_t value = base::ReadMemory<uint8_t>(address); // 1byteずつ読み取り
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

                if(dataLength < HEADER_SIZE + sizeof(uint32_t) + sizeof(uint8_t)) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                uint32_t startAddress = data.get<uint32_t>(dataIndex); // 書き込むメモリアドレスを取得
                dataIndex += sizeof(startAddress);
                bytes values; // 書き込む値のバッファ
                values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                uint32_t writeSize = values.size(); // 書き込むデータの長さ

                if(writeSize > freeMemorySize) // 書き込むデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }

                response.push_back(RESPONSE_SUCCESS);

                for(uint32_t i = 0; i < writeSize; i++) // すべてのデータを書き込むまでループ
                {
                    uint32_t address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        uint8_t value = values[i]; // 書き込む値
                        base::WriteMemory<uint8_t>(address, value); // アドレスに値を書き込む
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
                if(dataSize < DATA_HEDDER + (int32_t)sizeof(uint32_t) * 3) break;

                uint32_t startAddress = data.get<uint32_t>(DATA_HEDDER);  // 書き込むメモリアドレスを取得
                uint32_t searchSize = data.get<uint32_t>(DATA_HEDDER + (int32_t)sizeof(startAddress));
                uint32_t searchValue = data.get<uint32_t>(DATA_HEDDER + (int32_t)sizeof(startAddress) + (int32_t)sizeof(searchSize));

                bytes response;
            
                for(uint32_t i = 0; i < searchSize / sizeof(uint32_t); i++)
                {
                    uint32_t address = startAddress + i * sizeof(uint32_t);
                    if(OSIsAddressValid(address))
                    {
                        uint32_t value = ReadMemory<uint32_t>(address);
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
                uint32_t address = data.get<uint32_t>(dataIndex);
                dataIndex += sizeof(address);
                uint32_t size = data.get<uint32_t>(dataIndex);
                dataIndex += sizeof(size);
                uint8_t valueType = data.get<uint8_t>(dataIndex);

                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";

                response.append(size);

                switch (valueType)
                {
                    case VALUE_CHAR:
                    {
                        advanced::DumpMemory<int8_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UCHAR:
                    {
                        advanced::DumpMemory<uint8_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_SHORT:
                    {
                        advanced::DumpMemory<int16_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_USHORT:
                    {
                        advanced::DumpMemory<uint16_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_INT:
                    {
                        advanced::DumpMemory<int32_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT:
                    {
                        advanced::DumpMemory<uint32_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_FLOAT:
                    {
                        advanced::DumpMemory<float>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_DOUBLE:
                    {
                        advanced::DumpMemory<double>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    default:
                    {
                        RepsonseFailed(socket);
                        break;
                    }
                }
                break;
            }
            case COMMAND_SEARCH_UNKN:
            {
                // 開始アドレス(例:05 04 -> 前回のサーチ結果のサイズ分のメモリアドレスをunsigned intでサーチ)

                // コマンドから情報を取得
                uint8_t valueType = data.get<uint8_t>(dataIndex);
                dataIndex += sizeof(valueType);
                CompareFlags cmp = static_cast<CompareFlags>(data.get<uint8_t>(dataIndex));
            
                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";
            
                switch (valueType)
                {
                    case VALUE_CHAR:
                    {
                        MemoryList<int8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int8_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UCHAR:
                    {
                        MemoryList<uint8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint8_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_SHORT:
                    {
                        MemoryList<int16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int16_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_USHORT:
                    {
                        MemoryList<uint16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint16_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT:
                    {
                        MemoryList<int32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int32_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT:
                    {
                        MemoryList<uint32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint32_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_FLOAT:
                    {
                        MemoryList<float> src;
                        int32_t results;
                        results = advanced::SearchMemory<float>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_DOUBLE:
                    {
                        MemoryList<double> src;
                        int32_t results;
                        results = advanced::SearchMemory<double>(socket, dirPath, "Search", cmp);
                        response.append(results);
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