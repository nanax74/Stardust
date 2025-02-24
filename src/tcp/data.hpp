#pragma once

#include "../main.hpp"
#include "server.hpp"
#include "../utils/bytes.hpp"

namespace data
{
    void SendData(int socket, const bytes& data);   // データ送信関数
    bytes ReceiveData(int socket);  // データ受信関数
}