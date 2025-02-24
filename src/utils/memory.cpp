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
}