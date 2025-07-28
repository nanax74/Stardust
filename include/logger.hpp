#pragma once

#include <whb/log.h>
#include <whb/log_udp.h>

#include <string>
#include <format>

class Log
{
    public:
        static bool Init()
        {
            if(WHBLogUdpInit())
            {
                isEnabled = true;
                return true;
            }
            return false;
        }

        static bool Shutdown()
        {
            if(WHBLogUdpDeinit())
            {
                isEnabled = false;
                return true;
            }
            return false;
        }

        template<typename... Args>
        static bool Printf(std::format_string<Args...> fmt, Args&&... args);

    private:
        static inline bool isEnabled = false;
};

template <typename... Args>
bool Log::Printf(std::format_string<Args...> fmt, Args&&... args)
{
    if (!isEnabled) return false;
    try
    {
        std::string formatted = std::format(fmt, std::forward<Args>(args)...);
        std::string final_msg = std::format("[Stardust] {}", formatted);
        return WHBLogPrintf("%s", final_msg.c_str()) != 0;
    }
    catch (const std::format_error& e)
    {
        WHBLogPrintf("[Stardust][format error] %s", e.what());
        return false;
    }
}