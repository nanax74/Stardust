#pragma once

#include "../main.hpp"
#include <filesystem>
#include "server.hpp"
#include "data.hpp"
#include "../utils/memory.hpp"
#include "../utils/bytes.hpp"

namespace TCPServer
{
    void ProcessCommand(int socket, bytes& data);
}