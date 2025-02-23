#pragma once

#include "server.hpp"
#include "../utils/memory.hpp"
#include "../main.hpp"

namespace TCPServer
{
    void ProcessCommand(unsigned char* buffer, int length);
}