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
    int32_t SendAll(int32_t socket, Bytes& buffer);
    int32_t ReceiveAll(int32_t socket, Bytes& buffer);

    int32_t SendPacket(int32_t socket, Bytes& data, uint32_t type);
    int32_t ReceivePacket(int32_t socket, Bytes& data, uint32_t& type);

    class intEx // バイトオーダー変換ができる拡張クラス
    {
        private:
            int32_t value;

        public:
            intEx(int32_t v) : value(v) {}

            int32_t get();
            void set(int32_t v);

            intEx& reverse();

            // intExをintに変換
            operator int32_t() const;
        
            // intをintExに代入
            intEx& operator=(int32_t v);
    };
}