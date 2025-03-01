#pragma once

#include <string>

#define BUFFER_SIZE 0x100000 // バッファの大きさ

namespace memory
{
    unsigned char ReadUInt8(unsigned int address);
    unsigned short ReadUInt16(unsigned int address);
    unsigned int ReadUInt32(unsigned int address);
    void WriteUInt8(unsigned int address, unsigned char value);
    void WriteUInt16(unsigned int address, unsigned short value);
    void WriteUInt32(unsigned int address, unsigned int value);

    bool MemoryDump(std::string dirPath, unsigned int startAddress, unsigned int size);
    int SearchFromDump(std::string dirPath, unsigned int startAddress, unsigned int compareType);
    int SearchFromSearch(std::string dirPath, unsigned int startAddress, unsigned int compareType);
}