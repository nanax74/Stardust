#include "transfer.hpp"

// std
#include <algorithm>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

// wut
#include "whb/log.h"

// stardust
#include "../server/server.hpp"

using namespace std;

int32_t Transfer::SendAll(int32_t socket, Bytes& data)
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

int32_t Transfer::ReceiveAll(int32_t socket, Bytes& data)
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