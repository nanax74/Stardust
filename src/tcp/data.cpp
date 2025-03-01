#include "data.hpp"

// std
#include <algorithm>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

// wut
#include "whb/log.h"

// stardust
#include "server.hpp"

using namespace std;

namespace data
{
    #define DATA_LENGTH_MAX 0x10000

    bool SendData(int socket, bytes& data)
    {
        unsigned int dataLength = data.size(); // データの長さ
        if(!SendDataLength(socket, dataLength)) return false; // 受信失敗
        if(!SendDataContent(socket, data, dataLength)) return false; // 受信失敗
        return true; // 受信成功
    }

    bool ReceiveData(int socket, bytes& data)
    {
        unsigned int dataLength = 0; // データの長さ
        if(!ReceiveDataLength(socket, dataLength)) return false; // 受信失敗
        if(!ReceiveDataContent(socket, data, dataLength)) return false; // 受信失敗
        return true; // 受信成功
    }

    // データの長さを送信する関数
    bool SendDataLength(int socket, unsigned int dataLength)
    {
        if(dataLength == 0) // データの長さが0
        {
            WHBLogPrintf("Data length is zero");
            return false; // 送信失敗
        }

        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength); // 送信するためのポインター
        unsigned int totalLengthSent = 0; // 送信済みバイト数

        // 送信し終わるまでループ
        while(totalLengthSent < sizeof(dataLength))
        {
            int sent = send(socket, dataLengthPtr + totalLengthSent, sizeof(dataLength) - totalLengthSent, 0); // 送信
            if(sent <= 0) // 戻り値をチェック
            {
                if(sent == 0) // クライアントが切断
                {
                    WHBLogPrintf("Disconnected from client");
                    TCPServer::DisconnectClient();
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to send data length: %s", strerror(errno));
                }
                return false; // 送信失敗
            }
            totalLengthSent += sent; // インクリメント
        }
        return true; // 送信成功
    }

    // データの本体を送信する関数
    bool SendDataContent(int socket, bytes& data, unsigned int dataLength)
    {
        WHBLogPrintf("Sending:%dbytes", dataLength);
        if(data.empty()) // データが空
        {
            WHBLogPrintf("No data to send");
            return false; // 送信失敗
        }
        
        char* dataPtr = reinterpret_cast<char*>(data.data()); // 送信するためのポインター
        unsigned int totalSent = 0; // 送信済みバイト数

        // 送信し終わるまでループ
        while(totalSent < dataLength)
        {
            int sent = send(socket, dataPtr + totalSent, min(DATA_LENGTH_MAX, (int)dataLength - (int)totalSent), 0); // 送信
            if(sent <= 0) // 戻り値をチェック
            {
                if(sent == 0) // クライアントが切断
                {
                    WHBLogPrintf("Disconnected from client");
                    TCPServer::DisconnectClient();
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to send data: %s", strerror(errno));
                }
                return false; // 送信失敗
            }
            totalSent += sent; // インクリメント
        }
        WHBLogPrintf("Sent:%dbytes", dataLength);
        return true; // 送信成功
    }

    // データの長さを受信する関数
    bool ReceiveDataLength(int socket, unsigned int& dataLength)
    {
        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength); // 受信するためのポインター
        unsigned int totalLengthReceived = 0; // 受信済みバイト数

        // 受信し終わるまでループ
        while(totalLengthReceived < sizeof(dataLength))
        {
            int received = recv(socket, dataLengthPtr + totalLengthReceived, sizeof(dataLength) - totalLengthReceived, 0); // 受信
            if(received <= 0) // 戻り値をチェック
            {
                if(received == 0) // クライアントが切断
                {
                    WHBLogPrintf("Disconnected from client");
                    TCPServer::DisconnectClient();
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to receive data length: %s", strerror(errno));
                }
                return false; // 受信失敗
            }
            totalLengthReceived += received; // インクリメント
        }
        dataLength = static_cast<intEx>(dataLength).reverse().get(); // カスタムクラス:intExを使ってエンディアン反転
        return true; // 送信成功
    }

    // データの本体を受信する関数
    bool ReceiveDataContent(int socket, bytes& data, unsigned int dataLength)
    {
        data.resize(dataLength); // バッファをリサイズ
        char* dataPtr = reinterpret_cast<char*>(data.data()); // 受信するためのポインター
        unsigned int totalReceived = 0; // 受信済みバイト数

        // 受信し終わるまでループ
        while(totalReceived < dataLength)
        {
            int received = recv(socket, dataPtr + totalReceived, min(DATA_LENGTH_MAX, (int)dataLength - (int)totalReceived), 0); // 受信
            if(received <= 0) // 戻り値をチェック
            {
                if(received == 0) // クライアントが切断
                {
                    WHBLogPrintf("Disconnected from client");
                    TCPServer::DisconnectClient();
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to receive data: %s", strerror(errno));
                }
                return false; // 受信失敗
            }
            totalReceived += received; // インクリメント
        }
        return true; // 送信成功
    }

    // ファイルにバイナリデータを書き込む関数
    bool WriteFile(const string& filePath, const bytes& buffer)
    {
        FILE *file = fopen(filePath.c_str(), "ab");  // バイナリデータを追記
        if(!file) // ファイルが開けなかったら
        {
            WHBLogPrintf("Failed to open file: %s, errno: %d", filePath.c_str(), errno);
            return false; // 書き込み失敗
        }
    
        unsigned int bytesWritten = fwrite(buffer.data(), 1, buffer.size(), file); // 書き込み

        fclose(file); // ファイルを閉じる
    
        if(bytesWritten != buffer.size()) // バッファのサイズ分書き込まれなかったら
        {
            WHBLogPrintf("Failed to write all data. Expected: %zu, Written: %zu", buffer.size(), bytesWritten);
            return false; // 書き込み失敗
        }
    
        WHBLogPrintf("File writing successfully: %s (%zu bytes)", filePath.c_str(), buffer.size());
        return true; // 書き込み成功
    }

    bool ReadFile(const string& filePath, bytes& buffer, unsigned int offset, unsigned int dataLength) 
    {
        FILE* file = fopen(filePath.c_str(), "rb");  // バイナリデータを読み取る
        if(!file)
        {
            WHBLogPrintf("Failed to open file: %s, errno: %d", filePath.c_str(), errno);
            return false;
        }

        unsigned int fileSize = 0;
        GetFileSize(file, fileSize);
    
        // 読み取り範囲の調整
        if(offset >= (unsigned int)fileSize) // オフセットがファイルサイズを超えていたら読み取り不能
        {
            WHBLogPrintf("Offset exceeds file size: %s", filePath.c_str());
            fclose(file);
            return false;
        }
    
        if(offset + dataLength > (unsigned int)fileSize) // 終端を超える場合
        {
            dataLength = fileSize - offset; // 読み取れるサイズに調整
        }
    
        // 読み取り開始位置に移動
        if(fseek(file, offset, SEEK_SET) != 0)
        {
            WHBLogPrintf("Failed to seek to offset: %s", filePath.c_str());
            fclose(file);
            return false;
        }
    
        // 読み取り
        buffer.resize(dataLength);
        unsigned int bytesRead = fread(buffer.data(), 1, dataLength, file);
        fclose(file);
    
        if(bytesRead < dataLength)
        {
            WHBLogPrintf("Reached end of file. Expected: %u, Read: %u", dataLength, bytesRead);
            buffer.resize(bytesRead); // 実際に読み取れたサイズにリサイズ
        }
    
        WHBLogPrintf("File reading successfully: %s (%zu bytes)", filePath.c_str(), buffer.size());
        return true;
    }    

    // ファイルを削除
    bool RemoveFile(const string& filePath)
    {
        if(remove(filePath.c_str()) == 0)
        {
            WHBLogPrintf("%s is removed", filePath.c_str());
            return true;
        }
        else
        {
            WHBLogPrintf("Failed removing %s", filePath.c_str());
            return false;
        }
    }
    
    // ファイルの改名
    bool RenameFile(const string& filePath, const string& newFilePath)
    {
        if(std::rename(filePath.c_str(), newFilePath.c_str()) == 0)
        {
            WHBLogPrintf("Renamed: %s -> %s", filePath.c_str(), newFilePath.c_str());
            return true;
        }
        else
        {
            WHBLogPrintf("Failed to rename: %s -> %s, errno: %d", filePath.c_str(), newFilePath.c_str(), errno);
            return false;
        }
    }

    // ファイルの存在確認
    bool FileExists(const std::string &filePath)
    {
        struct stat buffer;
        return (stat(filePath.c_str(), &buffer) == 0);  // stat関数が成功すれば存在する
    }

    bool GetFileSize(const string& filePath, unsigned int& fileSize)
    {
        FILE *file = fopen(filePath.c_str(), "rb");  // ファイルを開く
        if(!file)
        {
            WHBLogPrintf("Failed to open file: %s, errno: %d", filePath.c_str(), errno);
            return false;
        }
    
        // ファイルサイズを取得
        if(fseek(file, 0, SEEK_END) != 0)
        {
            WHBLogPrintf("Failed to seek file: %s", filePath.c_str());
            fclose(file);
            return false;
        }
    
        fileSize = ftell(file);
        if(fileSize < 0)
        {
            WHBLogPrintf("Failed to get file size: %s", filePath.c_str());
            fclose(file);
            return false;
        }
        WHBLogPrintf("Size of %s is %d", filePath.c_str(), fileSize);
        fclose(file);
        return true;
    }

    bool GetFileSize(FILE* file, unsigned int& fileSize)
    {
        // ファイルサイズを取得
        if(fseek(file, 0, SEEK_END) != 0)
        {
            WHBLogPrintf("Failed to seek file");
            fclose(file);
            return false;
        }
    
        fileSize = ftell(file);
        if(fileSize < 0)
        {
            WHBLogPrintf("Failed to get file size");
            fclose(file);
            return false;
        }
        return true;
    }

    int intEx::get()
    {
        return value;
    }

    void intEx::set(int v)
    {
        value = v;
    }

    // バイトオーダー反転
    intEx& intEx::reverse()
    {
        unsigned char* bytePtr = reinterpret_cast<unsigned char*>(&value);
        std::reverse(bytePtr, bytePtr + sizeof(int));
        return *this;
    }

    // intExをintに変換
    intEx::operator int() const
    {
        return value;
    }

    // intをintExに代入
    intEx& intEx::operator=(int v)
    {
        value = v;
        return *this;
    }
}