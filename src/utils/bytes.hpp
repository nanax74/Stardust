#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

class Bytes
{
private:
    std::vector<uint8_t> _data;

public:
    Bytes(size_t size = 0)
    {
        _data = std::vector<uint8_t>(size);
    }

public:
    // 要素へのアクセス
    uint8_t& operator[](size_t index)
    {
        return _data[index];
    }

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
    T get(size_t index);

    size_t size() const
    {
        return _data.size();
    }

    bool empty() const
    {
        return _data.empty();
    }

    // 範囲指定
    void assign(auto first, auto last)
    {
        _data.assign(first, last);
    }

    auto begin()
    {
        return _data.begin();
    }

    auto end()
    {
        return _data.end();
    }

    void resize(size_t size)
    {
        _data.resize(size);
    }

    uint8_t* data()
    {
        return _data.data();
    }
};