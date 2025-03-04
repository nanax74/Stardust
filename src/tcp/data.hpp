#pragma once

// std
#include <string>

// stardust
#include "../utils/bytes.hpp"

#define MAX_DATA_SIZE 0x10000 // データ本体の最大サイズ

#define DATA_TYPE_COMMAND 0x01
#define DATA_TYPE_RESPONSE 0x02
#define DATA_TYPE_PROGRESS 0x03

namespace data
{
    int SendAll(int socket, bytes& buffer);
    int ReceiveAll(int socket, bytes& buffer);

    int SendPacket(int socket, bytes& data, unsigned int type);
    int ReceivePacket(int socket, bytes& data, unsigned int& type);

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