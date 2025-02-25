#include "data.hpp"

using namespace std;

namespace data
{
    // データ送信関数
    void SendData(int socket, bytes& data)
    {
        // データが空の場合中止
        if (data.empty())
        {
            WHBLogPrintf("No data to send.");
            return;
        }
    
        // データの長さを送信
        int dataLength = data.size();
        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength);
        int totalLengthSent = 0;
        while (totalLengthSent < (int)sizeof(dataLength))
        {
            int sent = send(socket, dataLengthPtr + totalLengthSent, sizeof(dataLength) - totalLengthSent, 0);
            if (sent <= 0) // send関数の戻り値をチェック
            {
                if (sent == 0)
                {
                    WHBLogPrintf("Connection closed by peer.");
                }
                else
                {
                    WHBLogPrintf("Failed to send data length: %s", strerror(errno));
                }
                return;
            }
            totalLengthSent += sent;
        }
        
    
        // データ本体を送信
        int totalSent = 0;
        char* dataPtr = reinterpret_cast<char*>(data.data());
        while (totalSent < dataLength)
        {
            int sent = send(socket, dataPtr + totalSent, min(4096, dataLength - totalSent), 0);
            if (sent <= 0) // send関数の戻り値をチェック
            {
                if (sent == 0)
                {
                    WHBLogPrintf("Connection closed by peer.");
                }
                else
                {
                    WHBLogPrintf("Failed to send data: %s", strerror(errno));
                }
                return;
            }
            totalSent += sent;
        }
    }

    // データ受信関数
    bytes ReceiveData(int socket)
    {
        // データの長さを受信
        int dataLength = 0;
        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength);
        int totalLengthReceived = 0;
        while (totalLengthReceived < (int)sizeof(dataLength))
        {
            int received = recv(socket, dataLengthPtr + totalLengthReceived, sizeof(dataLength) - totalLengthReceived, 0);
            if (received <= 0) // recv関数の戻り値をチェック
            {
                if (received == 0)
                {
                    WHBLogPrintf("Connection closed by peer.");
                }
                else
                {
                    WHBLogPrintf("Failed to receive data length: %s", strerror(errno));
                }
                return {}; // エラー時は空のデータを返す
            }
            totalLengthReceived += received;
        }
        dataLength = static_cast<intEx>(dataLength).reverse().get();    // カスタムクラス:intExを使ってエンディアン反転
        WHBLogPrintf("dataLength:%X", dataLength);
    
        // データ本体を受信
        bytes data;
        data.resize(dataLength);
        char* dataPtr = reinterpret_cast<char*>(data.data());
        int totalReceived = 0;
        while (totalReceived < dataLength)
        {
            int received = recv(socket, dataPtr + totalReceived, min(4096, dataLength - totalReceived), 0);
            if (received <= 0) // recv関数の戻り値をチェック
            {
                if (received == 0)
                {
                    WHBLogPrintf("Connection closed by peer.");
                }
                else
                {
                    WHBLogPrintf("Failed to receive data: %s", strerror(errno));
                }
                return {}; // エラー時は空のデータを返す
            }
            totalReceived += received;
        }
    
        return data;
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