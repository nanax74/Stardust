#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include <notifications/notifications.h>

#include "packet.hpp"

namespace Command
{
    // コマンドID
    enum class ID : uint32_t
    {
        // メモリ読み書き
        WriteMemory = 0x0,
        ReadMemory = 0x1,

        // メモリダンプ
        DumpMemory = 0x10,

        // メモリサーチ
        SpecificSearch = 0x20,
        UnknownSearch = 0x21,

        // ダミー
        Dummy = 0xFFFFFFFF
    };

    // 応答
    enum class Response : uint32_t
    {
        Success = 0x0,
        Failure = 0x1,
        Progress = 0x2
    };

    // ダミーハンドラ
    void Dummy(Packet& packet);
}

namespace Memory
{
    extern const std::filesystem::path STARDUST_PATH;

    using ProgressCallback = std::function<void(float progress)>;

    void SetProgressCallback(ProgressCallback callback);

    extern ProgressCallback progressCallback;

    constexpr size_t KB = 1024;
    constexpr size_t MB = 1024 * KB;
    constexpr size_t GB = 1024 * MB;
}
