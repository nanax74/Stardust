#pragma once

#include "packet.hpp"

namespace CommandDispatcher
{
    void RegisterHandlers();
    void Dispatch(Packet& packet);
}