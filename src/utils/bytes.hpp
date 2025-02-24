#pragma once

#include "../main.hpp"

class bytes : public std::vector<unsigned char>
{
    public:
        void append(unsigned char value);
        void append(unsigned short value);
        void append(unsigned int value);
        unsigned char getUChar(size_t offset);
        unsigned short getUShort(size_t offset);
        unsigned int getUInt(size_t offset);
};