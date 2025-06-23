#pragma once

#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

#include <notifications/notifications.h>

#include "packet.hpp"

namespace Command
{
    // 未知の値を検索するコマンド
    void UnknownSearch(Packet& packet);
}