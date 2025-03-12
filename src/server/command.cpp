#include "command.hpp"

// std
#include <cstdint>
#include <map>
#include <filesystem>

// wut
#include <coreinit/memorymap.h>
#include <whb/log.h>

// stardust
#include "../transfer/packet.hpp"
#include "../utils/compare.hpp"
#include "../utils/math.hpp"
#include "../utils/memory.hpp"

const uint8_t COMMAND_READ         = 0x00; // バイト数制限付き
const uint8_t COMMAND_WRITE        = 0x01; // バイト数制限付き
const uint8_t COMMAND_SEARCH_DUMP  = 0x02; // サーチ用のダンプ
const uint8_t COMMAND_SEARCH_SPEC  = 0x03; // ダンプからサーチ
const uint8_t COMMAND_SEARCH_UNKN  = 0x04; // ダンプからサーチ
const uint8_t COMMAND_WRITE_KERNEL = 0x05; // カーネル書き込み

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

using namespace std;
using namespace memory;

// コマンドの種類
map<uint8_t, string> commandMap =
{
    {COMMAND_READ, "Read"},
    {COMMAND_WRITE, "Write"},
    {COMMAND_SEARCH_SPEC, "Specific Search"},
    {COMMAND_SEARCH_DUMP, "Dump for Search"},
    {COMMAND_SEARCH_UNKN, "Unknown Search"},
    {COMMAND_WRITE_KERNEL, "Kernel Write"}
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
        Packet packet;
        packet.Data.append(RESPONSE_FAILED); // 失敗応答
        packet.Type = Packet::DATA_TYPE_DATA;
        packet.SendPacket(socket);
    }

    // コマンドを処理
    void ProcessCommand(int32_t socket, Bytes& data)
    {
        Bytes response;

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

                if(readSize > FREE_MEMORY_SIZE) // 読み取るデータが長過ぎれば中止
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
                Bytes values; // 書き込む値のバッファ
                values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                uint32_t writeSize = values.size(); // 書き込むデータの長さ

                if(writeSize > FREE_MEMORY_SIZE) // 書き込むデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }

                response.append(RESPONSE_SUCCESS);

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
                    case VALUE_INT8:
                    {
                        advanced::DumpMemory<int8_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT8:
                    {
                        advanced::DumpMemory<uint8_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_INT16:
                    {
                        advanced::DumpMemory<int16_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT16:
                    {
                        advanced::DumpMemory<uint16_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_INT32:
                    {
                        advanced::DumpMemory<int32_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT32:
                    {
                        advanced::DumpMemory<uint32_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_INT64:
                    {
                        advanced::DumpMemory<int64_t>(socket, dirPath, "Search", address, size);
                        break;
                    }
                    case VALUE_UINT64:
                    {
                        advanced::DumpMemory<uint64_t>(socket, dirPath, "Search", address, size);
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
            case COMMAND_SEARCH_SPEC:
            {
                // 値の種類 + 比較演算子 + サーチする値 (例:08 01 3F800000 -> 前回のサーチ結果と3F800000をfloatで比較し、一致するアドレスを検索)

                if(dataLength < HEADER_SIZE + sizeof(uint8_t) * 2 + sizeof(uint32_t)) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                uint8_t valueType = data.get<uint8_t>(dataIndex);
                dataIndex += sizeof(valueType);
                CompareFlags cmp = static_cast<CompareFlags>(data.get<uint8_t>(dataIndex));
                dataIndex += sizeof(cmp);

                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";
            
                switch (valueType)
                {
                    case VALUE_INT8:
                    {
                        int8_t value = data.get<int8_t>(dataIndex);
                        MemoryList<int8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int8_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT8:
                    {
                        uint8_t value = data.get<uint8_t>(dataIndex);
                        MemoryList<uint8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint8_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT16:
                    {
                        int16_t value = data.get<int16_t>(dataIndex);
                        MemoryList<int16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int16_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT16:
                    {
                        uint16_t value = data.get<uint16_t>(dataIndex);
                        MemoryList<uint16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint16_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT32:
                    {
                        int32_t value = data.get<int32_t>(dataIndex);
                        MemoryList<int32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int32_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT32:
                    {
                        uint32_t value = data.get<uint32_t>(dataIndex);
                        MemoryList<uint32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint32_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT64:
                    {
                        int64_t value = data.get<int64_t>(dataIndex);
                        MemoryList<int64_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int64_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT64:
                    {
                        uint64_t value = data.get<uint64_t>(dataIndex);
                        MemoryList<uint64_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint64_t>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_FLOAT:
                    {
                        float value = data.get<float>(dataIndex);
                        MemoryList<float> src;
                        int32_t results;
                        results = advanced::SearchMemory<float>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                    case VALUE_DOUBLE:
                    {
                        double value = data.get<double>(dataIndex);
                        MemoryList<double> src;
                        int32_t results;
                        results = advanced::SearchMemory<double>(socket, dirPath, "Search", cmp, value);
                        response.append(results);
                        break;
                    }
                }
                break;
            }
            case COMMAND_SEARCH_UNKN:
            {
                // 値の種類 + 比較演算子 (例:05 04 -> 前回のサーチ結果と現在のメモリをunsigned intで比較し、値が大きくなっているアドレスを検索)

                if(dataLength < HEADER_SIZE + sizeof(uint8_t) * 2) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }

                // コマンドから情報を取得
                uint8_t valueType = data.get<uint8_t>(dataIndex);
                dataIndex += sizeof(valueType);
                CompareFlags cmp = static_cast<CompareFlags>(data.get<uint8_t>(dataIndex));
            
                std::string dirPath = "fs:/vol/external01/wiiu/Stardust";
            
                switch (valueType)
                {
                    case VALUE_INT8:
                    {
                        MemoryList<int8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int8_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT8:
                    {
                        MemoryList<uint8_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint8_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT16:
                    {
                        MemoryList<int16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int16_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT16:
                    {
                        MemoryList<uint16_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint16_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT32:
                    {
                        MemoryList<int32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int32_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT32:
                    {
                        MemoryList<uint32_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint32_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_INT64:
                    {
                        MemoryList<int64_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<int64_t>(socket, dirPath, "Search", cmp);
                        response.append(results);
                        break;
                    }
                    case VALUE_UINT64:
                    {
                        MemoryList<uint64_t> src;
                        int32_t results;
                        results = advanced::SearchMemory<uint64_t>(socket, dirPath, "Search", cmp);
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
            case COMMAND_WRITE_KERNEL:
            {
                // 開始アドレス + 書き込む値 (例:0E000000 600000004E800020 -> 0x0E000000に[nop, blr]が書き込まれる)

                if(dataLength < HEADER_SIZE + sizeof(uint32_t) + sizeof(uint32_t)) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str());
                    RepsonseFailed(socket);
                    break;
                }
            
                uint32_t startAddress = data.get<uint32_t>(dataIndex); // 書き込むメモリアドレスを取得
                dataIndex += sizeof(startAddress);
                Bytes values; // 書き込む値のバッファ
                values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                uint32_t writeSize = values.size(); // 書き込むデータの長さ
            
                if(writeSize > FREE_MEMORY_SIZE) // 書き込むデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    RepsonseFailed(socket);
                    break;
                }
            
                response.append(RESPONSE_SUCCESS);
            
                for(uint32_t i = 0; i < writeSize; i += 4) // すべてのデータを書き込むまでループ
                {
                    uint32_t address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        uint32_t value = values.get<uint32_t>(i); // 書き込む値
                        base::WriteKernel(address, value); // アドレスに値を書き込む
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
            default:
            {
                break;
            }
        }
        Packet packet;
        packet.Data = response; // 応答
        packet.Type = Packet::DATA_TYPE_DATA;
        packet.SendPacket(socket);
        auto endTime = chrono::high_resolution_clock::now(); // 計測終了
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count(); // 経過時間を算出
        WHBLogPrintf("Processing complete: %lld ms", duration); // かかった時間を表示
    }
}