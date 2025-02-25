#pragma once

#include "../main.hpp"
#include "server.hpp"
#include "../utils/bytes.hpp"

namespace data
{
    void SendData(int socket, bytes& data);   // データ送信関数
    bytes ReceiveData(int socket);  // データ受信関数

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