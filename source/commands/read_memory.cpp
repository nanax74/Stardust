#include "commands/read_memory.hpp"
#include "commands/common.hpp"

namespace Command
{
    void ReadMemory(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ID::ReadMemory));

        if (packet.Size() < sizeof(uint32_t) * 2)
        {
            response.Write<uint32_t>(std::to_underlying(Response::Failure)); // 失敗
            response.Send();
            throw std::runtime_error("Packet too small to contain type and address");
        }

        uint32_t startAddress = packet.Read<uint32_t>();
        uint32_t endAddress   = packet.Read<uint32_t>();

        response.Write(startAddress);

        uint32_t currentAddress = startAddress;

        while (currentAddress < endAddress)
        {
            uint32_t remaining = endAddress - currentAddress;
        
            if (remaining >= 8)
            {
                uint64_t value = Memory::Read<uint64_t>(currentAddress);
                response.Write(value);
                currentAddress += 8;
            }
            else if (remaining >= 4)
            {
                uint32_t value = Memory::Read<uint32_t>(currentAddress);
                response.Write(value);
                currentAddress += 4;
            }
            else if (remaining >= 2)
            {
                uint16_t value = Memory::Read<uint16_t>(currentAddress);
                response.Write(value);
                currentAddress += 2;
            }
            else
            {
                uint8_t value = Memory::Read<uint8_t>(currentAddress);
                response.Write(value);
                currentAddress += 1;
            }
        }
        response.Send();
    }
}