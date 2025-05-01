#include "instruction.hpp"

// stardust
#include "../syscall/handler.hpp"

using namespace Syscall;

void SetDataBreakpoint(uint32_t address)
{
    asm volatile
    (
        "mr 3, %0\n" // ID
        "mr 4, %1\n" // アドレス
        "li 0, 0x6D00\n" // インデックス
        "sc\n"           // 呼び出し
        : // 出力
        : "r"(SET_DABR), "r"(address) // 入力
        : "r0", "r3", "r4", "memory" // 変更される領域
    );
}