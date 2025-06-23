#include "commands/common.hpp"

#include <filesystem>

namespace Command
{
    void Dummy(Packet& packet)
    {
        // ダミーのハンドラ。何もしない。
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ID::Dummy));
        response.Write<uint32_t>(std::to_underlying(Response::Success)); // 成功応答
        response.Send();
    }
}

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