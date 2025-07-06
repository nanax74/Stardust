#pragma once

#include "packet.hpp"

#include <cstdint>
#include <format>
#include <stdexcept>
#include <type_traits>

#include <coreinit/memorymap.h>

#include <notifications/notifications.h>

namespace Command
{
    // メモリ書き込みコマンド
    void WriteMemory(Packet& packet);
}

namespace Memory
{
    template <typename T>
    void Write(uint32_t address, const T& value)
    {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");

        // アドレスが有効かチェック
        if (!OSIsAddressValid(address))
        {
            throw std::runtime_error("Invalid memory address");
        }

        // メモリに書き込み
        T* ptr = reinterpret_cast<T*>(address);
        *ptr = value;
    }
}