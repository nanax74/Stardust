#include "bytes.hpp"

using namespace std;

// charをdataに追加
void bytes::append(int8_t value)
{
    this->push_back(value);  // そのまま1バイトを追加
}

// unsigned charをdataに追加
void bytes::append(uint8_t value)
{
    this->push_back(value);  // そのまま1バイトを追加
}

// shortを2バイトに分けてdataに追加
void bytes::append(int16_t value)
{
    // 2バイト分のデータを追加
    for(int32_t i = 0; i < 2; ++i)
    {
        this->push_back((value >> (8 * (1 - i))) & 0xFF);  // バイトごとに分けて追加
    }
}

// unsigned shortを2バイトに分けてdataに追加
void bytes::append(uint16_t value)
{
    // 2バイト分のデータを追加
    for(int32_t i = 0; i < 2; ++i)
    {
        this->push_back((value >> (8 * (1 - i))) & 0xFF);  // バイトごとに分けて追加
    }
}

// intを4バイトに分けてdataに追加
void bytes::append(int32_t value)
{
    // 4バイト分のデータを追加
    for(int32_t i = 0; i < 4; ++i)
    {
        this->push_back((value >> (8 * (3 - i))) & 0xFF); // バイトごとに分けて追加
    }
}

// unsigned intを4バイトに分けてdataに追加
void bytes::append(uint32_t value)
{
    // 4バイト分のデータを追加
    for(int32_t i = 0; i < 4; ++i)
    {
        this->push_back((value >> (8 * (3 - i))) & 0xFF); // バイトごとに分けて追加
    }
}

// longを8バイトに分けてdataに追加
void bytes::append(long value)
{
    // 4バイト分のデータを追加
    for(int32_t i = 0; i < 8; ++i)
    {
        this->push_back((value >> (8 * (7 - i))) & 0xFF); // バイトごとに分けて追加
    }
}

// unsigned longを8バイトに分けてdataに追加
void bytes::append(unsigned long value)
{
    // 4バイト分のデータを追加
    for(int32_t i = 0; i < 8; ++i)
    {
        this->push_back((value >> (8 * (7 - i))) & 0xFF); // バイトごとに分けて追加
    }
}