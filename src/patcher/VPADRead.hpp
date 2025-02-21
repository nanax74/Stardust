#pragma once

#include <vpad/input.h>
#include "../main.hpp"
#include "../tcp/server.hpp"

namespace VPAD
{
    void Start(VPADStatus status);
    void Update(VPADStatus status);
}