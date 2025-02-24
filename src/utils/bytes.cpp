#include "bytes.hpp"

using namespace std;

// unsigned charをdataに追加
void bytes::append(unsigned char value)
{
    this->push_back(value);  // そのまま1バイトを追加
}

// unsigned shortを2バイトに分けてdataに追加
void bytes::append(unsigned short value)
{
    // 2バイト分のデータを追加
    for (int i = 0; i < 2; ++i)
    {
        this->push_back((value >> (8 * (1 - i))) & 0xFF);  // バイトごとに分けて追加
    }
}

// unsigned intを4バイトに分けてdataに追加
void bytes::append(unsigned int value)
{
    // 4バイト分のデータを追加
    for (int i = 0; i < 4; ++i)
    {
        this->push_back((value >> (8 * (3 - i))) & 0xFF); // バイトごとに分けて追加
    }
}

// 特定のオフセットから1バイトを取得してunsigned charに変換
unsigned char bytes::getUChar(size_t offset)
{
    if (offset >= this->size()) return 0;

    return this->at(offset);
}

// 特定のオフセットから2バイトを取得してunsigned shortに変換
unsigned short bytes::getUShort(size_t offset)
{
    if (offset + 2 > this->size()) return 0;

    unsigned short result = 0;
    for (int i = 0; i < 2; ++i)
    {
        result |= (this->at(offset + i) << (8 * (1 - i)));  // ビッグエンディアン形式で変換
    }

    return result;
}

// 特定のオフセットから4バイト取得してunsigned intに変換
unsigned int bytes::getUInt(unsigned int offset)
{
    if (offset + 4 > this->size()) return 0;

    unsigned int result = 0;
    for (int i = 0; i < 4; ++i)
    {
        result |= (this->at(offset + i) << (8 * (3 - i)));  // ビッグエンディアン形式で変換
    }

    return result;
}