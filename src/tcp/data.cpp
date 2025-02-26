#include "data.hpp"

using namespace std;

namespace data
{
    bool SendData(int socket, bytes& data)
    {
        int dataLength = data.size(); // データの長さ
        if(!SendDataLength(socket, dataLength)) return false; // 受信失敗
        if(!SendDataContent(socket, data, dataLength)) return false; // 受信失敗
        return true; // 受信成功
    }

    bool ReceiveData(int socket, bytes& data)
    {
        int dataLength = 0; // データの長さ
        if(!ReceiveDataLength(socket, dataLength)) return false; // 受信失敗
        if(!ReceiveDataContent(socket, data, dataLength)) return false; // 受信失敗
        return true; // 受信成功
    }

    // データの長さを送信する関数
    bool SendDataLength(int socket, int& dataLength)
    {
        if(dataLength == 0) // データの長さが0
        {
            WHBLogPrintf("Data length is zero");
            return false; // 送信失敗
        }

        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength); // 送信するためのポインター
        int totalLengthSent = 0; // 送信済みバイト数

        // 送信し終わるまでループ
        while (totalLengthSent < (int)sizeof(dataLength))
        {
            int sent = send(socket, dataLengthPtr + totalLengthSent, sizeof(dataLength) - totalLengthSent, 0); // 送信
            if (sent <= 0) // 戻り値をチェック
            {
                if (sent == 0) // クライアントが切断
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
    bool SendDataContent(int socket, bytes& data, int dataLength)
    {
        if (data.empty()) // データが空
        {
            WHBLogPrintf("No data to send");
            return false; // 送信失敗
        }
        
        char* dataPtr = reinterpret_cast<char*>(data.data()); // 送信するためのポインター
        int totalSent = 0; // 送信済みバイト数

        // 送信し終わるまでループ
        while (totalSent < dataLength)
        {
            int sent = send(socket, dataPtr + totalSent, min(4096, dataLength - totalSent), 0); // 送信
            if (sent <= 0) // 戻り値をチェック
            {
                if (sent == 0) // クライアントが切断
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
        WHBLogPrintf("Sent:%dbytes", data.size());
        return true; // 送信成功
    }

    // データの長さを受信する関数
    bool ReceiveDataLength(int socket, int& dataLength)
    {
        char* dataLengthPtr = reinterpret_cast<char*>(&dataLength); // 受信するためのポインター
        int totalLengthReceived = 0; // 受信済みバイト数

        // 受信し終わるまでループ
        while (totalLengthReceived < (int)sizeof(dataLength))
        {
            int received = recv(socket, dataLengthPtr + totalLengthReceived, sizeof(dataLength) - totalLengthReceived, 0); // 受信
            if (received <= 0) // 戻り値をチェック
            {
                if (received == 0) // クライアントが切断
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
    bool ReceiveDataContent(int socket, bytes& data, int dataLength)
    {
        data.resize(dataLength); // バッファをリサイズ
        char* dataPtr = reinterpret_cast<char*>(data.data()); // 受信するためのポインター
        int totalReceived = 0; // 受信済みバイト数

        // 受信し終わるまでループ
        while (totalReceived < dataLength)
        {
            int received = recv(socket, dataPtr + totalReceived, min(4096, dataLength - totalReceived), 0); // 受信
            if (received <= 0) // 戻り値をチェック
            {
                if (received == 0) // クライアントが切断
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