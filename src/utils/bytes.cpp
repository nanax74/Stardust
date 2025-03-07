#include "bytes.hpp"

using namespace std;

void bytes::append(int8_t value)
{
    this->push_back(value);
}

void bytes::append(uint8_t value)
{
    this->push_back(value);
}

void bytes::append(int16_t value)
{
    for(int32_t i = 0; i < 2; ++i)
    {
        this->push_back((value >> (8 * (1 - i))) & 0xFF);
    }
}

void bytes::append(uint16_t value)
{
    for(int32_t i = 0; i < 2; ++i)
    {
        this->push_back((value >> (8 * (1 - i))) & 0xFF);
    }
}

void bytes::append(int32_t value)
{
    for(int32_t i = 0; i < 4; ++i)
    {
        this->push_back((value >> (8 * (3 - i))) & 0xFF);
    }
}

void bytes::append(uint32_t value)
{
    for(int32_t i = 0; i < 4; ++i)
    {
        this->push_back((value >> (8 * (3 - i))) & 0xFF);
    }
}

void bytes::append(int64_t value)
{
    for(int32_t i = 0; i < 8; ++i)
    {
        this->push_back((value >> (8 * (7 - i))) & 0xFF);
    }
}

void bytes::append(uint64_t value)
{
    for(int32_t i = 0; i < 8; ++i)
    {
        this->push_back((value >> (8 * (7 - i))) & 0xFF);
    }
}

void bytes::append(float value)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
    for (int i = 0; i < 4; ++i)
    {
        this->push_back(ptr[i]);
    }
}

void bytes::append(double value)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
    for (int i = 0; i < 8; ++i)
    {
        this->push_back(ptr[i]);
    }
}
