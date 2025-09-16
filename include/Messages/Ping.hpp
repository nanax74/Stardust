#pragma once

#include <StardustLib/MessageServer.hpp>
#include "messages/Pong.hpp"

namespace Stardust
{
    class Ping : public StardustLib::MessageBase
    {
    private:
        uint32_t value = 0;

    public:
        Ping(uint32_t clientId, std::shared_ptr<StardustLib::TCPServer> server) : MessageBase(clientId, server) {}

        void serialize(StardustLib::BufferWriter& writer) const override
        {
            writer.write(value);
        }

        void deserialize(StardustLib::BufferReader& reader) override
        {
            value = reader.read<uint32_t>();
        }

        void process() override
        {
            Pong pong(getClientId(), getServer());
            pong.send();
        }
    };
}