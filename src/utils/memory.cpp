#include "memory.hpp"

namespace memory
{
    unsigned char ReadUInt8(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned char*)address;
        }
        return 0;
    }
    
    unsigned short ReadUInt16(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned short*)address;
        }
        return 0;
    }
    
    unsigned int ReadUInt32(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned int*)address;
        }
        return 0;
    }

    void WriteUInt8(unsigned int address, unsigned char value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned char*)address = value;
        }
    }

    void WriteUInt16(unsigned int address, unsigned short value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned short*)address = value;
        }
    }
    void WriteUInt32(unsigned int address, unsigned int value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned int*)address = value;
        }
    }
}