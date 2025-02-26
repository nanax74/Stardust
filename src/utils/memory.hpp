#pragma once

#include <coreinit/memorymap.h>
#include "../main.hpp"

namespace memory
{
    unsigned char ReadUInt8(unsigned int address);
    unsigned short ReadUInt16(unsigned int address);
    unsigned int ReadUInt32(unsigned int address);
    void WriteUInt8(unsigned int address, unsigned char value);
    void WriteUInt16(unsigned int address, unsigned short value);
    void WriteUInt32(unsigned int address, unsigned int value);
}