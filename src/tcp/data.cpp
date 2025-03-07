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
    int32_t SendAll(int32_t socket, Bytes& data)
    {
        uint32_t size = data.size(); // 送信するデータのサイズ

        WHBLogPrintf("Sending:%dbytes", size);
        
        uint32_t sent = 0; // 送信済みバイト数

        // 送信し終わるまでループ
        while(sent < size)
        {
            int32_t bytes = send(socket, data.data() + sent, size - sent, 0); // 送信
            if(bytes <= 0) // 戻り値をチェック
            {
                if(bytes == 0) // クライアントが切断
                {
                    WHBLogPrintf("Disconnected from client");
                    return 0;
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to send data: %s", strerror(errno));
                }
                return -1; // 送信失敗
            }
            sent += bytes; // インクリメント
        }
        WHBLogPrintf("Sent:%dbytes", sent);
        return sent; // 送信成功
    }

    int32_t ReceiveAll(int32_t socket, Bytes& data)
    {
        uint32_t size = data.size(); // 受信するデータのサイズ

        WHBLogPrintf("Receiving:%dbytes", size);

        uint32_t received = 0; // 受信済みバイト数

        // 受信し終わるまでループ
        while(received < size)
        {
            int32_t bytes = recv(socket, data.data() + received, size - received, 0); // 受信
            if(bytes <= 0) // 戻り値をチェック
            {
                if(bytes == 0) // クライアントが切断
                {   
                    WHBLogPrintf("Disconnected from client");
                    return 0;
                }
                else // 例外
                {
                    WHBLogPrintf("Failed to receive data: %s", strerror(errno));
                }
                return -1; // 受信失敗
            }
            received += bytes; // インクリメント
        }
        WHBLogPrintf("Received:%dbytes", received);
        return received; // 送信成功
    }

    int32_t SendPacket(int32_t socket, Bytes& data, uint32_t type)
    {
        uint32_t size = data.size();

        if(data.empty()) // データが空
        {
            WHBLogPrintf("Data to send is empty");
            return -1;
        }
        else if(size > MAX_DATA_SIZE) // データが大きすぎる
        {
            WHBLogPrintf("Data to send is too large:%dbytes", size);
            return -1;
        }

        Bytes header;
        header.append(type);
        header.append(size);

        int32_t result = SendAll(socket, header); // ヘッダを送信
        if(result <= 0) return result; // 送信失敗

        return SendAll(socket, data); // データを送信
    }

    int32_t ReceivePacket(int32_t socket, Bytes& data, uint32_t& type)
    {
        size_t headerSize = sizeof(uint32_t) * 2; // データのサイズ

        Bytes header; 
        header.resize(headerSize);

        int32_t result = ReceiveAll(socket, header);
        if(result <= 0) return result; // 受信失敗

        type = header.get<uint32_t>(0);
        uint32_t length = header.get<uint32_t>(sizeof(type));

        if(length > MAX_DATA_SIZE)
        {
            WHBLogPrintf("Data too large to receive:%dbytes", length);
            return -1; // データが大きすぎる
        }

        if(length == 0) return 0;

        data.resize(length);

        return ReceiveAll(socket, data);
    }

    int32_t intEx::get()
    {
        return value;
    }

    void intEx::set(int32_t v)
    {
        value = v;
    }

    // バイトオーダー反転
    intEx& intEx::reverse()
    {
        uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&value);
        std::reverse(bytePtr, bytePtr + sizeof(int32_t));
        return *this;
    }

    // intExをintに変換
    intEx::operator int32_t() const
    {
        return value;
    }

    // intをintExに代入
    intEx& intEx::operator=(int32_t v)
    {
        value = v;
        return *this;
    }
}