#pragma once

#include <cstdint>
#include <vector>

class bytes : public std::vector<uint8_t>
{
    public:
        void append(int8_t value);
        void append(uint8_t value);
        void append(int16_t value);
        void append(uint16_t value);
        void append(int32_t value);
        void append(uint32_t value);
        void append(int64_t value);
        void append(uint64_t value);
        void append(float value);
        void append(double value);

        // 特定のオフセット値を取得
        template <typename T>
        T get(size_t offset)
        {
            if(offset + sizeof(T) > this->size()) return 0;
        
            T result = 0;
            for(size_t i = 0; i < sizeof(T); ++i)
            {
                result |= (static_cast<T>(this->at(offset + i)) << (8 * (sizeof(T) - 1 - i)));  // ビッグエンディアン形式で変換
            }
        
            return result;
        }
};