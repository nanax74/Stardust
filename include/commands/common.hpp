#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>

#include <notifications/notifications.h>

namespace Command
{
    // コマンドIDの定義
    constexpr uint32_t ID_WriteMemory = 0;
    constexpr uint32_t ID_SpecificSearch = 1;

    // 応答
    constexpr uint32_t Response_Success = 0;
    constexpr uint32_t Response_Failure = 1;
    constexpr uint32_t Response_Progress = 2;
}

namespace Memory
{
    extern const std::filesystem::path STARDUST_PATH;

    using ProgressCallback = std::function<void(float progress)>;

    void SetProgressCallback(ProgressCallback callback);

    extern ProgressCallback progressCallback;
}
