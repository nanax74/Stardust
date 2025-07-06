#include "read_multiple_memory.hpp"

#include <vector>
#include <whb/log.h>
#include <coreinit/cache.h> // for DCFlushRange
#include <coreinit/memorymap.h>

#include "commands/common.hpp"

namespace Command
{
    void ReadMultipleMemory(Packet& packet)
    {
        WHBLogPrintf("ReadMultipleMemory command received.");

        // Read the number of memory regions
        uint32_t numRegions = packet.Read<uint32_t>();

        Packet response(packet.GetContext());
        response.Write(static_cast<uint32_t>(Command::ID::ReadMultipleMemory));
        response.Write(static_cast<uint32_t>(Command::Response::Data));
        response.Write(numRegions); // Write back the number of regions

        for (uint32_t i = 0; i < numRegions; ++i)
        {
            uint32_t address = packet.Read<uint32_t>();
            uint32_t size = packet.Read<uint32_t>();

            bool isValidRange = true;
            if (size == 0 || address == 0 || !OSIsAddressValid(address) || !OSIsAddressValid(address + size - 1))
            {
                isValidRange = false;
                WHBLogPrintf("ReadMultipleMemory: Invalid address or size or range (address: 0x%X, size: %u)", address, size);
            }

            // Write address and size back to the response
            response.Write(address);
            response.Write(size);

            if (!isValidRange)
            {
                // Fill with zeros for invalid data
                for (uint32_t j = 0; j < size; ++j)
                {
                    response.Write<uint8_t>(0);
                }
                continue;
            }

            // Ensure address is cache-line aligned for DCFlushRange
            uint32_t alignedAddress = Memory::AlignDown(address, 32);
            uint32_t alignedSize = Memory::AlignUp(size + (address - alignedAddress), 32);

            // Flush data cache for the region to ensure we read from memory, not stale cache
            DCFlushRange(reinterpret_cast<void*>(alignedAddress), alignedSize);

            // Read memory and append to response
            for (uint32_t j = 0; j < size; ++j)
            {
                response.Write<uint8_t>(*reinterpret_cast<uint8_t*>(address + j));
            }
        }
        response.Send();
        WHBLogPrintf("ReadMultipleMemory command finished.");
    }
}