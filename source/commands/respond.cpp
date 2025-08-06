#include "commands/respond.hpp"
#include "commands/common.hpp"

#include <filesystem>

namespace Command
{
    void Respond::Process(Packet& packet)
    {
        Packet response(packet.GetContext());
        response.Write(std::to_underlying(ClientCommand::Respond));
        response.Send();
    }
}