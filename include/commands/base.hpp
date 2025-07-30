#pragma once

#include "packet.hpp"
#include "logger.hpp"

#include <string>

namespace Command
{
    class Base
    {
        public:
            Base(std::string name) : name(name) {}

            void Execute(Packet& packet)
            {
                Log::Printf("[{}] received", name);
                Process(packet);
            }

            virtual void Process(Packet& packet) = 0;

        protected:
            void Log(std::string msg)
            {
                Log::Printf("[{}] {}", name, msg);
            }

        private:
            std::string name;
    };
}