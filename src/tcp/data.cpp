#include "data.hpp"

using namespace std;

namespace data
{
    // データ送信関数
    void SendData(int socket, const bytes& data)
    {
        // データの長さを送信
        int dataLength = data.size();
        int lengthSent = send(socket, reinterpret_cast<const char*>(&dataLength), sizeof(dataLength), 0);
        if (lengthSent == -1)
        {
            WHBLogPrintf("Failed to send data length.");
            return;
        }

        // データ本体を送信
        int totalSent = 0;
        while (totalSent < dataLength)
        {
            int sent = send(socket, reinterpret_cast<const char*>(data.data()) + totalSent, min(4096, dataLength - totalSent), 0);
            if (sent == -1)
            {
                WHBLogPrintf("Failed to send data.");
                return;
            }
            totalSent += sent;
        }
    }

    // データ受信関数
    bytes ReceiveData(int socket)
    {
        // まずデータの長さを受信
        int dataLength = 0;
        int lengthReceived = recv(socket, reinterpret_cast<char*>(&dataLength), sizeof(dataLength), 0);
        if (lengthReceived == -1)
        {
            WHBLogPrintf("Failed to receive data length.");
            return {};
        }

        // 受信するデータを格納するためのバッファを準備
        bytes data;
        int totalReceived = 0;
        while (totalReceived < dataLength)
        {
            int received = recv(socket, reinterpret_cast<char*>(data.data()) + totalReceived, min(4096, dataLength - totalReceived), 0);
            if (received == -1)
            {
                WHBLogPrintf("Failed to receive data.");
                return {};
            }
            totalReceived += received;
        }

        return data;
    }
}