#pragma once

// std
#include <cstdint>

// wut
#include <whb/log.h>

// stardust
#include "implement.hpp"

namespace Syscall
{
    const uint32_t SET_DABR = 0;
    // システムコール経由で関数を呼び出すためのハンドラー
    //
    // 呼び出し例:
    //     li r0, 0x6D00  // syscall index
    //     li r3, 0       // id
    //     sc
    uint32_t Handler(uint32_t id, uint32_t arg1);
}