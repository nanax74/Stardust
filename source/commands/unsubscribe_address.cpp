#include "commands/unsubscribe_address.hpp"
#include "subscribed_address.hpp"

namespace Command
{
    void UnsubscribeAddress::Process(Packet& packet)
    {
        auto client = packet.GetContext();
        if (!client) return;

        ClearSubscribedAddresses(client);
    }
}
