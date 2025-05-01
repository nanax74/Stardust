#pragma once

#include <cstdint>

const uint32_t nop = 0x60000000;
const uint32_t blr = 0x4E800020;

void SetDataBreakpoint(uint32_t address);