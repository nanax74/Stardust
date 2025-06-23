#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>

#include <notifications/notifications.h>

namespace Memory
{
    extern const std::filesystem::path STARDUST_PATH;

    using ProgressCallback = std::function<void(uint32_t currentAddress, float progress)>;

    void SetProgressCallback(ProgressCallback callback);

    extern ProgressCallback progressCallback;
}
