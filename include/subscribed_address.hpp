#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <map>
#include <memory>

#include "commands/common.hpp"
#include "socket_context.hpp"


struct SubscribedAddress
{
    uint32_t address;
    Command::ValueType type;
    uint64_t last_value;
};

extern std::map<std::shared_ptr<SocketContext>, std::vector<SubscribedAddress>> g_subscribed_addresses;
extern std::mutex g_subscribed_addresses_mutex;

void CheckSubscribedAddresses();
void ClearSubscribedAddresses(std::shared_ptr<SocketContext> client);
