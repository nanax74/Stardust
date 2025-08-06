#include "commands/subscribe_address.hpp"
#include "subscribed_address.hpp"

namespace Command
{
    void SubscribeAddress::Process(Packet& packet)
    {
        auto client = packet.GetContext();
        if (!client) return;

        uint32_t count = packet.Read<uint32_t>();
        std::vector<SubscribedAddress> addresses;
        for (uint32_t i = 0; i < count; ++i)
        {
            SubscribedAddress sub;
            sub.address = packet.Read<uint32_t>();
            sub.type = packet.Read<ValueType>();
            sub.last_value = 0; // 初期値
            addresses.push_back(sub);
        }

        std::lock_guard<std::mutex> lock(g_subscribed_addresses_mutex);
        g_subscribed_addresses[client] = addresses;
    }
}
