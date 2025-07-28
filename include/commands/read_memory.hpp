#pragma once

#include "commands/base.hpp"
#include "packet.hpp"

#include <coreinit/memorymap.h>

namespace Command
{
    class ReadMemory : public Base
    {
        public:
            ReadMemory() : Base("ReadMemory") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    template <typename T>
    T Read(uint32_t address)
    {
        static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");

        // アドレスが有効かチェック
        if (!OSIsAddressValid(address))
        {
            throw std::runtime_error("Invalid memory address");
        }

        // メモリに書き込み
        return *reinterpret_cast<T*>(address);
    }
}