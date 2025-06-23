#pragma once

#include <unordered_map>
#include <functional>
#include <cstdint>

#include "packet.hpp"

#include "commands/common.hpp"

class CommandDispatcher
{
public:
    using HandlerFunc = std::function<void(Packet&)>;

    static void RegisterHandlers(); // 起動時に呼び出す
    static void Dispatch(Packet& packet);

private:
    static std::unordered_map<Command::ID, HandlerFunc> handlers;
};
