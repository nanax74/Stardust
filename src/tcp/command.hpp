#pragma once

#include "server.hpp"
#include "../main.hpp"

namespace TCPServer
{
    void ProcessCommand(unsigned char* buffer, int length);
}