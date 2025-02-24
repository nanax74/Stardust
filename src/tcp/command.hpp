#pragma once

#include "server.hpp"
#include "data.hpp"
#include "../utils/memory.hpp"
#include "../utils/bytes.hpp"
#include "../main.hpp"

namespace TCPServer
{
    void ProcessCommand(int socket, bytes& data);
}