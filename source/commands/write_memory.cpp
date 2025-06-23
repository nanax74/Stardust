#include "commands/write_memory.hpp"

#include <format>

#include <notifications/notifications.h>

namespace Command
{
    void WriteMemory(Packet& packet)
    {
        if (packet.Size() < sizeof(uint32_t) * 2)
        {
            throw std::runtime_error("Packet too small to contain type and address");
        }
    
        uint32_t type = packet.Read<uint32_t>();
        uint32_t address = packet.Read<uint32_t>();
    
        switch (type)
        {
            case 1: // uint8_t
            {
                if (packet.Size() < sizeof(uint8_t))
                    throw std::runtime_error("Packet too small for uint8_t value");
                uint8_t value = packet.Read<uint8_t>();
                Memory::Write(address, value);
                break;
            }
            case 2: // uint16_t
            {
                if (packet.Size() < sizeof(uint16_t))
                    throw std::runtime_error("Packet too small for uint16_t value");
                uint16_t value = packet.Read<uint16_t>();
                Memory::Write(address, value);
                break;
            }
            case 4: // uint32_t
            {
                if (packet.Size() < sizeof(uint32_t))
                    throw std::runtime_error("Packet too small for uint32_t value");
                uint32_t value = packet.Read<uint32_t>();
                Memory::Write(address, value);
                break;
            }
            case 8: // uint64_t
            {
                if (packet.Size() < sizeof(uint64_t))
                    throw std::runtime_error("Packet too small for uint64_t value");
                uint64_t value = packet.Read<uint64_t>();
                Memory::Write(address, value);
                break;
            }
            default:
                throw std::runtime_error("Unsupported value type");
        }
    }
}