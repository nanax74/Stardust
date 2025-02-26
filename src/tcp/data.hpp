#pragma once

#include "../main.hpp"
#include "server.hpp"
#include "../utils/bytes.hpp"

namespace data
{
    bool SendData(int socket, bytes& data);   // データ送信関数
    bool ReceiveData(int socket, bytes& data);  // データ受信関数

    bool SendDataLength(int socket, int& dataLength); // データの長さを送信する関数
    bool SendDataContent(int socket, bytes& data, int dataLength); // データの本体を送信する関数
    bool ReceiveDataLength(int socket, int& dataLength); // データの長さを受信する関数
    bool ReceiveDataContent(int socket, bytes& data, int dataLength); // データの本体を受信する関数

    class intEx // バイトオーダー変換ができる拡張クラス
    {
        private:
            int value;

        public:
            intEx(int v) : value(v) {}

            int get();
            void set(int v);

            intEx& reverse();

            // intExをintに変換
            operator int() const;
        
            // intをintExに代入
            intEx& operator=(int v);
    };
}