#pragma once

#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class UnsubscribeAddress : public Base
    {
        public:
            UnsubscribeAddress() : Base("UnsubscribeAddress") {}
            void Process(Packet& packet) override;
    };
}