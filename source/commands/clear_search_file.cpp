#include "commands/clear_search_file.hpp"
#include "commands/common.hpp"

#include <filesystem>

namespace Command
{
    void ClearSearchFile::Process(Packet& packet)
    {
        Memory::ClearSearchFile();

        // 成功応答を返す
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ClientCommand::ClearSearchFile));
        response.Write<uint32_t>(std::to_underlying(Response::Success));
        response.Send();
    }
}

namespace Memory
{
    void ClearSearchFile()
    {
        namespace fs = std::filesystem;

        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        // 念のため一時ファイルも削除
        fs::remove(basePath / "Value.tmp");
        fs::remove(basePath / "Address.tmp");
        fs::remove(basePath / "Value.bin");
        fs::remove(basePath / "Address.bin");
        fs::remove(basePath / "Search.txt");
    }
}