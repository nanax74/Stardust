#include "command.hpp"

using namespace std;
using namespace memory;
using namespace data;

#define DATA_HEDDER 1   // ヘッダーの大きさ

#define BUFFER_SIZE 0x100000 // バッファの大きさ

#define COMMAND_READ 0x00 // バイト数制限付き
#define COMMAND_WRITE 0x01 // バイト数制限付き
#define COMMAND_DUMP 0x03 // バイト数無制限、読み取ったデータは"fs:/vol/external01/wiiu/Stardust/Dump.bin"に保存
#define COMMAND_SEARCH 0x02 // 結果が1MBを超えた場合、結果を送信せずにサイズだけ送信、結果はファイルに保存

#define RESPONSE_SUCCESS 0x00 // 成功応答
#define RESPONSE_VALUE 0x01 // 値応答
#define RESPONSE_FAILED 0xFF // 失敗応答

// コマンドの種類
map<unsigned char, string> commandMap =
{
    {COMMAND_READ, "Read"},
    {COMMAND_WRITE, "Write"},
    {COMMAND_DUMP, "Dump"},
    {COMMAND_SEARCH, "Search"}
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
    // 失敗応答を送信
    void ResponseFailed(int socket)
    {
        bytes response;
        response.append((unsigned char)RESPONSE_FAILED);
        SendData(socket, response);
    }

    // コマンドを処理
    void ProcessCommand(int socket, bytes& data)
    {
        int dataSize = data.size(); // コマンドのサイズ

        if(dataSize == 0) return; // コマンドのサイズが0なら中止

        unsigned char command = data[0];  // コマンドの種類

        string commandStr = GetCommandTypeStr(command); // コマンドの種類を文字列に変換
        WHBLogPrintf("CommandType:%s", commandStr.c_str());  // コマンドの種類を出力

        // 受け取ったコマンドを文字列として出力
        ostringstream os;
        for(int i = 0; i < dataSize; i++) // コマンドの長さの分だけ繰り返す
        {
            os << hex << setw(2) << setfill('0') << (int)data[i]; // 文字列ストリームにバイトを追加
        }
        WHBLogPrintf("Command:%s", os.str().c_str()); // 出力

        switch (command) // コマンドの種類に応じて処理
        {
            case COMMAND_READ: // データ読み取り
            {
                // 開始アドレス + 読み取る長さ (例:1234567800001234 -> 12345678から1234byte読み取られる)
                if (dataSize < DATA_HEDDER + (int)sizeof(unsigned int) * 2) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str()); // コマンドが短すぎます
                    ResponseFailed(socket); // コマンドの長さが足りない場合は失敗応答を送信
                    break;
                }

                unsigned int startAddress = data.getUInt(DATA_HEDDER);  // 読み取るアドレスを取得
                unsigned int readSize = data.getUInt(DATA_HEDDER + (int)sizeof(unsigned int)); // 読み取るサイズを取得

                if(readSize > BUFFER_SIZE) // 読み取るデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    ResponseFailed(socket); // 失敗応答を送信
                    break;
                }

                bytes response; // 応答
                response.push_back(RESPONSE_VALUE); // 値応答

                for(unsigned int i = 0; i < readSize; i++) // すべてのデータを読み取るまでループ
                {
                    unsigned int address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        unsigned char value = ReadUInt8(address); // 1byteずつ読み取り
                        response.append(value); // 応答に追加
                    }
                    else // 不正なアドレス
                    {
                        WHBLogPrintf("%s is Failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                        ResponseFailed(socket); // 失敗応答
                        break;
                    }
                }
                SendData(socket, response); // 結果を送信

                WHBLogPrintf("%s is Success!", commandStr.c_str()); // 成功！

                break; // 終了
            }
            case COMMAND_WRITE:
            {
                // 開始アドレス + 書き込む値 (例:123456780123456789ABCDEF -> 12345678に8byteの値0123456789ABCDEFが書き込まれる)
                if(dataSize < DATA_HEDDER + (int)sizeof(unsigned int) + (int)sizeof(unsigned char)) // コマンドの長さが足りなければ中止
                {
                    WHBLogPrintf("%s is Failed: Command is too short", commandStr.c_str()); // コマンドが短すぎます
                    ResponseFailed(socket); // コマンドの長さが足りない場合は失敗応答を送信
                    break;
                }

                unsigned int startAddress = data.getUInt(DATA_HEDDER); // 書き込むメモリアドレスを取得
                bytes values; // 書き込む値のバッファ
                values.assign(data.begin() + DATA_HEDDER + (int)sizeof(unsigned int), data.end());  // 書き込む値を取得
                unsigned int writeSize = values.size(); // 書き込むデータの長さ

                if(writeSize > BUFFER_SIZE) // 書き込むデータが長過ぎれば中止
                {
                    WHBLogPrintf("%s is Failed: Data is too long", commandStr.c_str()); // データが長すぎます
                    ResponseFailed(socket); // 失敗応答
                    break;
                }

                bytes response; // 応答

                for(unsigned int i = 0; i < writeSize; i++) // すべてのデータを書き込むまでループ
                {
                    unsigned int address = startAddress + i; // 現在のアドレス
                    if(OSIsAddressValid(address)) // アドレスが有効
                    {
                        unsigned char value = values[i]; // 書き込む値
                        WriteUInt8(address, value); // アドレスに値を書き込む
                    }
                    else // 不正なアドレス
                    {
                        WHBLogPrintf("%s is Failed: Address is invalid", commandStr.c_str()); // アドレスが無効です
                        ResponseFailed(socket); // 失敗応答
                        break;
                    }
                }
                SendData(socket, response); // 結果を送信
                break;
            }
            case COMMAND_SEARCH:
            {
                // 開始アドレス + サーチする長さ + サーチする値 (例:10000000 00001000 3F800000 -> 10000000から1000byteの範囲で値が3F800000のアドレスが検索される)
                if(dataSize < DATA_HEDDER + (int)sizeof(unsigned int) * 3) break;

                unsigned int startAddress = data.getUInt(DATA_HEDDER);  // 書き込むメモリアドレスを取得
                unsigned int searchSize = data.getUInt(DATA_HEDDER + (int)sizeof(startAddress));
                unsigned int searchValue = data.getUInt(DATA_HEDDER + (int)sizeof(startAddress) + (int)sizeof(searchSize));

                bytes response;
            
                for(unsigned int i = 0; i < searchSize / sizeof(unsigned int); i++)
                {
                    unsigned int address = startAddress + i * sizeof(unsigned int);
                    if(OSIsAddressValid(address))
                    {
                        unsigned int value = ReadUInt32(address);
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
                break;
            }
            case COMMAND_DUMP:
            {
                if(dataSize < DATA_HEDDER + (int)sizeof(unsigned int) * 2) break; // コマンドの長さが足りなければ中止
            
                auto startTime = std::chrono::high_resolution_clock::now(); // 計測開始

                string filePath = "fs:/vol/external01/wiiu/Stardust/Dump.bin"; // ダンプ結果を保存するファイル

                RemoveFile(filePath); // ファイルがある場合は削除してから開始

                unsigned int startAddress = data.getUInt(DATA_HEDDER);
                unsigned int readSize = data.getUInt(DATA_HEDDER + (int)sizeof(unsigned int));
            
                unsigned int totalSent = 0;
                bytes buffer;
                buffer.resize(BUFFER_SIZE); // バッファを事前確保
            
                while(totalSent < readSize)
                {
                    unsigned int currentStartAddress = startAddress + totalSent;
                    unsigned int remainingSize = readSize - totalSent;
                    unsigned int readChunkSize = min(BUFFER_SIZE, (int)remainingSize);
            
                    if(OSIsAddressValid(currentStartAddress) && OSIsAddressValid(currentStartAddress + readChunkSize - 1))
                    {
                        WHBLogPrintf("Copying Data:%X~%X", currentStartAddress, currentStartAddress + readChunkSize - 1);
                        memcpy(buffer.data(), (void*)currentStartAddress, readChunkSize);
                        WHBLogPrintf("Copied Data:%X~%X", currentStartAddress, currentStartAddress + readChunkSize - 1);
                        WriteFile(filePath, buffer);
                        totalSent += readChunkSize;
                    }
                    else
                    {
                        break; // 不正アドレスがあれば終了
                    }
                }

                auto endTime = std::chrono::high_resolution_clock::now(); // 計測終了
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count(); // 経過時間を算出
                
                bytes response;
                response.append(readSize);
                SendData(socket, response); // 応答

                WHBLogPrintf("Processing complete: %lld ms", duration); // かかった時間を表示

                break;
            }
            default:
            {
                break;
            }
        }
    }
}