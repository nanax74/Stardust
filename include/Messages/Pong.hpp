#pragma once

#include <StardustLib/MessageServer.hpp>

namespace Stardust
{
    class Pong : public StardustLib::MessageBase
    {
    private:
        uint32_t value = 0;

    public:
        Pong(uint32_t clientId, std::shared_ptr<StardustLib::TCPServer> server) : MessageBase(clientId, server) {}

        void serialize(StardustLib::BufferWriter& writer) const override
        {
            writer.write(value);
        }

        void deserialize(StardustLib::BufferReader& reader) override
        {
            value = reader.read<uint32_t>();
        }
    };
}