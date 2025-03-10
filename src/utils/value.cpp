#include "value.hpp"

// std
#include <bit>
#include <cstring>
#include <cstdint>

template <typename T>
T Value::Reverse(T value)
{
    return std::byteswap(value);
}

template int8_t Value::Reverse(int8_t value);
template uint8_t Value::Reverse(uint8_t value);
template int16_t Value::Reverse(int16_t value);
template uint16_t Value::Reverse(uint16_t value);
template int32_t Value::Reverse(int32_t value);
template uint32_t Value::Reverse(uint32_t value);
template int64_t Value::Reverse(int64_t value);
template uint64_t Value::Reverse(uint64_t value);