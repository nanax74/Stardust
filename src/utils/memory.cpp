#include "memory.hpp"

unsigned char ReadUInt8(unsigned int address)
{
    if(OSIsAddressValid(address))
    {
        return *(unsigned char*)address;
    }
}

unsigned short ReadUInt16(unsigned int address)
{
    if(OSIsAddressValid(address))
    {
        return *(unsigned short*)address;
    }
}

unsigned int ReadUInt32(unsigned int address)
{
    if(OSIsAddressValid(address))
    {
        return *(unsigned int*)address;
    }
}