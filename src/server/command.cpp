#include "command.hpp"

// std
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>

// wut
#include <coreinit/memorymap.h>
#include <whb/log.h>

// stardust
#include "../assembly/instruction.hpp"
#include "../exception/callback.hpp"
#include "../syscall/implement.hpp"
#include "../transfer/packet.hpp"
#include "../utils/compare.hpp"
#include "../utils/math.hpp"
#include "../utils/memory.hpp"

namespace Command
{
    // コマンドを処理
    void ProcessCommand(int32_t socket, Bytes& data)
    {
        auto startTime = std::chrono::high_resolution_clock::now(); // タイマー計測開始

        try
        {
            uint32_t dataLength = data.size(); // コマンドの長さ
            if(dataLength == 0) throw "Command is empty"; // コマンドが空です

            uint8_t command = data[0];  // コマンドの種類
            std::string commandStr = GetCommandTypeStr(command); // コマンドの種類を文字列に変換
            WHBLogPrintf("Command Type: %s", commandStr.c_str());  // コマンドの種類を出力
            PrintCommandLog(dataLength, data); // コマンドを出力

            uint32_t dataIndex = HEADER_SIZE;

            Bytes response; // 応答のバッファ

            switch (command) // コマンドの種類に応じて処理
            {
                case COMMAND_READ: // データ読み取り
                {
                    // 開始アドレス + 読み取る長さ (例:10000000 00001000 -> 0x10000000から0x1000byte読み取られる)

                    if(dataLength < sizeof(uint32_t) * 2) // コマンドの長さが足りなければ中止
                    {
                        WHBLogPrintf("%s failed: Command is too short", commandStr.c_str());
                        SendError(socket, startTime, ERROR_COMMAND_INVALID);
                        return;
                    }

                    uint32_t startAddress = data.get<uint32_t>(dataIndex);  // 読み取るアドレスを取得
                    dataIndex += sizeof(startAddress);
                    uint32_t readSize = data.get<uint32_t>(dataIndex); // 読み取るサイズを取得

                    if(readSize > memory::FREE_MEMORY_SIZE) // 読み取るデータが長過ぎれば中止
                    {
                        WHBLogPrintf("%s failed: Data is too long", commandStr.c_str()); // データが長すぎます
                        SendError(socket, startTime, ERROR_DATA_TOO_LONG);
                        return;
                    }

                    for(uint32_t i = 0; i < readSize; i++) // すべてのデータを読み取るまでループ
                    {
                        uint32_t address = startAddress + i; // 現在のアドレス
                        if(OSIsAddressValid(address)) // アドレスが有効
                        {
                            uint8_t value = memory::base::ReadMemory<uint8_t>(address); // 1byteずつ読み取り
                            response.append(value); // 応答に追加
                        }
                        else // 不正なアドレス
                        {
                            WHBLogPrintf("%s failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                            SendError(socket, startTime, ERROR_ADDRESS_INVALID);
                            return;
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
                        SendError(socket, startTime, ERROR_COMMAND_INVALID);
                        return;
                    }

                    uint32_t startAddress = data.get<uint32_t>(dataIndex); // 書き込むメモリアドレスを取得
                    dataIndex += sizeof(startAddress);
                    Bytes values; // 書き込む値のバッファ
                    values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                    uint32_t writeSize = values.size(); // 書き込むデータの長さ

                    if(writeSize > memory::FREE_MEMORY_SIZE) // 書き込むデータが長過ぎれば中止
                    {
                        WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                        SendError(socket, startTime, ERROR_DATA_TOO_LONG);
                        return;
                    }

                    response.append(RESPONSE_SUCCESS);

                    for(uint32_t i = 0; i < writeSize; i++) // すべてのデータを書き込むまでループ
                    {
                        uint32_t address = startAddress + i; // 現在のアドレス
                        if(OSIsAddressValid(address)) // アドレスが有効
                        {
                            uint8_t value = values[i]; // 書き込む値
                            memory::base::WriteMemory<uint8_t>(address, value); // アドレスに値を書き込む
                        }
                        else // 不正なアドレス
                        {
                            WHBLogPrintf("%s is Failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                            SendError(socket, startTime, ERROR_ADDRESS_INVALID);
                            return;
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
                            memory::advanced::DumpMemory<int8_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_UINT8:
                        {
                            memory::advanced::DumpMemory<uint8_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_INT16:
                        {
                            memory::advanced::DumpMemory<int16_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_UINT16:
                        {
                            memory::advanced::DumpMemory<uint16_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_INT32:
                        {
                            memory::advanced::DumpMemory<int32_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_UINT32:
                        {
                            memory::advanced::DumpMemory<uint32_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_INT64:
                        {
                            memory::advanced::DumpMemory<int64_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_UINT64:
                        {
                            memory::advanced::DumpMemory<uint64_t>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_FLOAT:
                        {
                            memory::advanced::DumpMemory<float>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        case VALUE_DOUBLE:
                        {
                            memory::advanced::DumpMemory<double>(socket, dirPath, "Search", address, size);
                            break;
                        }
                        default:
                        {
                            SendError(socket, startTime, ERROR_VALUE_TYPE_INVALID); // 無効な値の型
                            WHBLogPrintf("%s is Failed: Invalid value type", commandStr.c_str()); // 無効な値の型です
                            return;
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
                        SendError(socket, startTime, ERROR_COMMAND_INVALID);
                        return;
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
                            memory::MemoryList<int8_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int8_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT8:
                        {
                            uint8_t value = data.get<uint8_t>(dataIndex);
                            memory::MemoryList<uint8_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint8_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT16:
                        {
                            int16_t value = data.get<int16_t>(dataIndex);
                            memory::MemoryList<int16_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int16_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT16:
                        {
                            uint16_t value = data.get<uint16_t>(dataIndex);
                            memory::MemoryList<uint16_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint16_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT32:
                        {
                            int32_t value = data.get<int32_t>(dataIndex);
                            memory::MemoryList<int32_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int32_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT32:
                        {
                            uint32_t value = data.get<uint32_t>(dataIndex);
                            memory::MemoryList<uint32_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint32_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT64:
                        {
                            int64_t value = data.get<int64_t>(dataIndex);
                            memory::MemoryList<int64_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int64_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT64:
                        {
                            uint64_t value = data.get<uint64_t>(dataIndex);
                            memory::MemoryList<uint64_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint64_t>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_FLOAT:
                        {
                            float value = data.get<float>(dataIndex);
                            memory::MemoryList<float> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<float>(socket, dirPath, "Search", cmp, value);
                            response.append(results);
                            break;
                        }
                        case VALUE_DOUBLE:
                        {
                            double value = data.get<double>(dataIndex);
                            memory::MemoryList<double> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<double>(socket, dirPath, "Search", cmp, value);
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
                        SendError(socket, startTime, ERROR_COMMAND_INVALID);
                        return;
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
                            memory::MemoryList<int8_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int8_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT8:
                        {
                            memory::MemoryList<uint8_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint8_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT16:
                        {
                            memory::MemoryList<int16_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int16_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT16:
                        {
                            memory::MemoryList<uint16_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint16_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT32:
                        {
                            memory::MemoryList<int32_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int32_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT32:
                        {
                            memory::MemoryList<uint32_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint32_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_INT64:
                        {
                            memory::MemoryList<int64_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<int64_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_UINT64:
                        {
                            memory::MemoryList<uint64_t> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<uint64_t>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_FLOAT:
                        {
                            memory::MemoryList<float> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<float>(socket, dirPath, "Search", cmp);
                            response.append(results);
                            break;
                        }
                        case VALUE_DOUBLE:
                        {
                            memory::MemoryList<double> src;
                            int32_t results;
                            results = memory::advanced::SearchMemory<double>(socket, dirPath, "Search", cmp);
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
                        SendError(socket, startTime, ERROR_COMMAND_INVALID);
                        return;
                    }
                
                    uint32_t startAddress = data.get<uint32_t>(dataIndex); // 書き込むメモリアドレスを取得
                    dataIndex += sizeof(startAddress);
                    Bytes values; // 書き込む値のバッファ
                    values.assign(data.begin() + dataIndex, data.end());  // 書き込む値を取得
                    uint32_t writeSize = values.size(); // 書き込むデータの長さ
                
                    if(writeSize > memory::FREE_MEMORY_SIZE) // 書き込むデータが長過ぎれば中止
                    {
                        WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                        SendError(socket, startTime, ERROR_DATA_TOO_LONG);
                        return;
                    }
                
                    response.append(RESPONSE_SUCCESS);
                
                    for(uint32_t i = 0; i < writeSize; i += 4) // すべてのデータを書き込むまでループ
                    {
                        uint32_t address = startAddress + i; // 現在のアドレス
                        if(OSIsAddressValid(address)) // アドレスが有効
                        {
                            uint32_t value = values.get<uint32_t>(i); // 書き込む値
                            memory::base::WriteKernel(address, value); // アドレスに値を書き込む
                        }
                        else // 不正なアドレス
                        {
                            WHBLogPrintf("%s is Failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                            SendError(socket, startTime, ERROR_ADDRESS_INVALID);
                            return;
                        }
                    }
                    break;
                }
                case COMMAND_SET_DATA_BREAKPOINT:
                {
                    // アドレス(例:15000000 -> 0x15000000にDataBreakpointを設定する)

                    if(dataLength < HEADER_SIZE + sizeof(uint32_t)) // コマンドの長さが足りなければ中止
                    {
                        throw std::format("{} failed: Command is too short", commandStr.c_str());
                        return;
                    }

                    uint32_t value = data.get<uint32_t>(dataIndex); // 書き込むメモリアドレスを取得
                    SetDataBreakpoint(value); // DABRにアドレスを書き込む
                    break;
                }
                case COMMAND_SEND_CONTEXT: // Breakpoint発動時に取得したコンテキストをまとめて送信する
                {
                    // コマンドIDのみでOK
                    SetContextPacket(response);
                    break;
                }
                default:
                {
                    break;
                }
            }

            if(response.size() == 0)
            {
                throw format("{} failed: No response", commandStr); // 応答がない場合は例外を投げる
            }

            SendSuccess(socket, startTime, response);
        }
        catch(std::string e)
        {
            WHBLogPrintf(e.c_str()); // 例外を表示
            SendError(socket, startTime, ERROR_DATA_EMPTY); // 応答がなければ失敗
        }
    }

    void SendSuccess(int socket, auto startTime, Bytes response)
    {
        Packet packet;
        packet.Data = response; // 応答
        packet.Type = Packet::DATA_TYPE_DATA;
        packet.SendPacket(socket);
        auto endTime = std::chrono::high_resolution_clock::now(); // 計測終了
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); // 経過時間を算出
        WHBLogPrintf("Processing complete: %lld ms", duration); // かかった時間を表示
    }

    void SendError(int socket, auto startTime, uint8_t errorCode)
    {
        Packet packet;
        packet.Data = Bytes();
        packet.Data.append(errorCode);
        packet.Type = Packet::DATA_TYPE_NULL;
        packet.SendPacket(socket);
        auto endTime = std::chrono::high_resolution_clock::now(); // 計測終了
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); // 経過時間を算出
        WHBLogPrintf("Processing failed: %lld ms", duration); // かかった時間を表示
    }

    const std::map<uint8_t, std::string> commandNameMap =
    {
        {COMMAND_READ, "Read"},
        {COMMAND_WRITE, "Write"},
        {COMMAND_SEARCH_SPEC, "Specific Search"},
        {COMMAND_SEARCH_DUMP, "Dump for Search"},
        {COMMAND_SEARCH_UNKN, "Unknown Search"},
        {COMMAND_WRITE_KERNEL, "Kernel Write"},
        {COMMAND_SET_DATA_BREAKPOINT, "Set Data Breakpoint"},
        {COMMAND_SEND_CONTEXT, "Send Context"},
    };
    
    // コマンドの種類を文字列で取得
    std::string GetCommandTypeStr(uint8_t command)
    {
        auto it = commandNameMap.find(command);
        if(it != commandNameMap.end())
        {
            return it->second;
        }
        return "Unknown";
    }

    void PrintCommandLog(uint32_t length, Bytes& data)
    {
        // 受け取ったコマンドを文字列として出力
        std::ostringstream os;
        for(uint32_t i = 0; i < length; i++) // コマンドの長さの分だけ繰り返す
        {
            os << std::hex << std::setw(2) << std::setfill('0') << (int32_t)data[i]; // 文字列ストリームにバイトを追加
        }
        WHBLogPrintf("Command:%s", os.str().c_str()); // 出力
    }

    void SetContextPacket(Bytes& response)
    {
        for(OSContext context : Exception::contextBuffer)
        {
            response.append(context.srr0);
        }
        Exception::contextBuffer.clear();
    }
};