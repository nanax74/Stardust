#pragma once

// std
#include <string>

// stardust
#include "../utils/bytes.hpp"

class Transfer
{
public:
    static int32_t SendAll(int32_t socket, Bytes& buffer);
    static int32_t ReceiveAll(int32_t socket, Bytes& buffer);
};