#include "subscribed_address.hpp"
#include "server.hpp"
#include "packet.hpp"

std::map<std::shared_ptr<SocketContext>, std::vector<SubscribedAddress>> g_subscribed_addresses;
std::mutex g_subscribed_addresses_mutex;

// subscribed_address.cpp
void CheckSubscribedAddresses()
{
    std::lock_guard<std::mutex> lock(g_subscribed_addresses_mutex);

    for (auto& pair : g_subscribed_addresses)
    {
        auto client = pair.first;
        auto& addresses = pair.second;

        if (!client || !client->connected)
        {
            continue;
        }

        for (auto& sub : addresses)
        {
            uint64_t current_value = 0;
            bool success = true;

            switch (sub.type)
            {
                case Command::ValueType::Int8:
                {
                    int8_t val = *reinterpret_cast<int8_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::UInt8:
                {
                    uint8_t val = *reinterpret_cast<uint8_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::Int16:
                {
                    int16_t val = *reinterpret_cast<int16_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::UInt16:
                {
                    uint16_t val = *reinterpret_cast<uint16_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::Int32:
                {
                    int32_t val = *reinterpret_cast<int32_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::UInt32:
                case Command::ValueType::Float:
                {
                    uint32_t val = *reinterpret_cast<uint32_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::Int64:
                {
                    int64_t val = *reinterpret_cast<int64_t*>(sub.address);
                    current_value = val;
                    break;
                }
                case Command::ValueType::UInt64:
                case Command::ValueType::Double:
                {
                    uint64_t val = *reinterpret_cast<uint64_t*>(sub.address);
                    current_value = val;
                    break;
                }
                default:
                    success = false;
                    break;
            }

            if (success && current_value != sub.last_value)
            {
                sub.last_value = current_value;

                Packet packet(client);
                packet.Write(static_cast<uint32_t>(Command::ServerCommand::PushAddressValue));
                packet.Write(sub.address);
                packet.Write(static_cast<uint32_t>(sub.type));
                packet.Write(current_value);
                packet.Send();
            }
        }
    }
}

void ClearSubscribedAddresses(std::shared_ptr<SocketContext> client)
{
    std::lock_guard<std::mutex> lock(g_subscribed_addresses_mutex);
    g_subscribed_addresses.erase(client);
}
