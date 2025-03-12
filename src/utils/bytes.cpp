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

// 特定のオフセット値を取得
template <typename T>
T Bytes::get(size_t index)
{
    if(index + sizeof(T) > _data.size()) return 0;

    T result = 0;
    for(size_t i = 0; i < sizeof(T); ++i)
    {
        result |= (static_cast<T>(_data.at(index + i)) << (8 * (sizeof(T) - 1 - i)));  // ビッグエンディアン形式で変換
    }

    return result;
}

template int8_t Bytes::get<int8_t>(size_t index);
template uint8_t Bytes::get<uint8_t>(size_t index);
template int16_t Bytes::get<int16_t>(size_t index);
template uint16_t Bytes::get<uint16_t>(size_t index);
template int32_t Bytes::get<int32_t>(size_t index);
template uint32_t Bytes::get<uint32_t>(size_t index);
template int64_t Bytes::get<int64_t>(size_t index);
template uint64_t Bytes::get<uint64_t>(size_t index);

template<>
float Bytes::get<float>(size_t index)
{
    if(index + sizeof(float) > _data.size()) return 0;
    
    uint32_t result = 0;
    for(size_t i = 0; i < sizeof(uint32_t); ++i)
    {
        result |= (static_cast<uint32_t>(_data.at(index + i)) << (8 * (sizeof(uint32_t) - 1 - i)));  // ビッグエンディアン形式で変換
    }
    return static_cast<float>(result);
}

template<>
double Bytes::get<double>(size_t index)
{
    if(index + sizeof(double) > _data.size()) return 0;
    
    uint64_t result = 0;
    for(size_t i = 0; i < sizeof(uint64_t); ++i)
    {
        result |= (static_cast<uint64_t>(_data.at(index + i)) << (8 * (sizeof(uint64_t) - 1 - i)));  // ビッグエンディアン形式で変換
    }
    return static_cast<double>(result);
}