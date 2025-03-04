#pragma once

#include <type_traits>

enum class CompareFlags : unsigned char
{
    NONE = 0x00,
    EQ = 0x01, // Equal
    NE = 0x02, // Not Equal
    GT = 0x04, // Greater than
    GE = 0x08, // Greater than or equal
    LE = 0x10, // Less than or equal
    LT = 0x20  // Less than
};

// ビット演算のオーバーロード
constexpr CompareFlags operator|(CompareFlags a, CompareFlags b)
{
    return static_cast<CompareFlags>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
}

constexpr CompareFlags& operator|=(CompareFlags& a, CompareFlags b)
{
    return a = a | b;
}

constexpr CompareFlags operator&(CompareFlags a, CompareFlags b)
{
    return static_cast<CompareFlags>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
}

constexpr CompareFlags& operator&=(CompareFlags& a, CompareFlags b)
{
    return a = a & b;
}

template <typename T>
CompareFlags Compare(T a, T b)
{
    CompareFlags flags = CompareFlags::NONE;

    if (a == b) flags |= CompareFlags::EQ;
    if (a != b) flags |= CompareFlags::NE;
    if (a > b)  flags |= CompareFlags::GT;
    if (a >= b) flags |= CompareFlags::GE;
    if (a <= b) flags |= CompareFlags::LE;
    if (a < b)  flags |= CompareFlags::LT;

    return flags;
}