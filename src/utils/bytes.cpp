#include "bytes.hpp"

using namespace std;

void Bytes::append(int8_t value)
{
    _data.push_back(value);
}

void Bytes::append(uint8_t value)
{
    _data.push_back(value);
}

void Bytes::append(int16_t value)
{
    for(int32_t i = 0; i < 2; ++i)
    {
        _data.push_back((value >> (8 * (1 - i))) & 0xFF);
    }
}

void Bytes::append(uint16_t value)
{
    for(int32_t i = 0; i < 2; ++i)
    {
        _data.push_back((value >> (8 * (1 - i))) & 0xFF);
    }
}

void Bytes::append(int32_t value)
{
    for(int32_t i = 0; i < 4; ++i)
    {
        _data.push_back((value >> (8 * (3 - i))) & 0xFF);
    }
}

void Bytes::append(uint32_t value)
{
    for(int32_t i = 0; i < 4; ++i)
    {
        _data.push_back((value >> (8 * (3 - i))) & 0xFF);
    }
}

void Bytes::append(int64_t value)
{
    for(int32_t i = 0; i < 8; ++i)
    {
        _data.push_back((value >> (8 * (7 - i))) & 0xFF);
    }
}

void Bytes::append(uint64_t value)
{
    for(int32_t i = 0; i < 8; ++i)
    {
        _data.push_back((value >> (8 * (7 - i))) & 0xFF);
    }
}

void Bytes::append(float value)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
    for (int i = 0; i < 4; ++i)
    {
        _data.push_back(ptr[i]);
    }
}

void Bytes::append(double value)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
    for (int i = 0; i < 8; ++i)
    {
        _data.push_back(ptr[i]);
    }
}
