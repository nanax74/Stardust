#include "commands/clear_search_file.hpp"
#include "common.hpp"

#include <filesystem>

namespace Command
{
    void ClearSearchFile(Packet& packet)
    {
        Memory::ClearSearchFile();
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