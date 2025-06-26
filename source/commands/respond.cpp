#include "commands/respond.hpp"
#include "common.hpp"

#include <filesystem>

namespace Command
{
    void Respond(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write(std::to_underlying(ID::Respond));
        response.Send();
    }
}