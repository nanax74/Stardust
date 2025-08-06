#pragma once

#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class SubscribeAddress : public Base
    {
        public:
            SubscribeAddress() : Base("SubscribeAddress") {}
            void Process(Packet& packet) override;
    };
}