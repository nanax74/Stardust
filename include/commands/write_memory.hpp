#pragma once

#include "commands/base.hpp"
#include "packet.hpp"

#include <cstdint>
#include <format>
#include <stdexcept>
#include <type_traits>

#include <coreinit/memorymap.h>

#include "notification.hpp"

namespace Command
{
    class WriteMemory : public Base
    {
        public:
            WriteMemory() : Base("WriteMemory") {}
            void Process(Packet& packet) override;
    };
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