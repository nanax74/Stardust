#include "commands/common.hpp"

#include <filesystem>

namespace Memory
{
    const std::filesystem::path STARDUST_PATH = "fs:/vol/external01/wiiu/Stardust";

    // コールバックの実体（ヘッダで extern されたもの）
    ProgressCallback progressCallback = nullptr;

    void SetProgressCallback(ProgressCallback callback)
    {
        progressCallback = callback;
    }
}