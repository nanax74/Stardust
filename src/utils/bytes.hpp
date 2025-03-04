#pragma once

#include <vector>

class bytes : public std::vector<unsigned char>
{
    public:
        void append(char value);
        void append(unsigned char value);
        void append(short value);
        void append(unsigned short value);
        void append(int value);
        void append(unsigned int value);
        void append(long value);
        void append(unsigned long value);

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