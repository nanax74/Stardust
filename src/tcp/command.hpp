#pragma once

#include "server.hpp"
#include "../utils/memory.hpp"
#include "../utils/bytes.hpp"
#include "../main.hpp"

namespace TCPServer
{
    void ProcessCommand(int socket, bytes& data);
}