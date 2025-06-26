#include "commands/check_search_file.hpp"
#include "common.hpp"

#include <filesystem>

namespace Command
{
    void CheckSearchFile(Packet& packet)
    {
        uint32_t exists = Memory::CheckSearchFile() == true ? 1 : 0;
        Packet response(packet.GetContext());
        response.Write(std::to_underlying(ID::CheckSearchFile));
        response.Write(exists);
        response.Send();
    }
}

namespace Memory
{
    bool CheckSearchFile()
    {
        namespace fs = std::filesystem;

        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        // ファイルの存在確認
        bool valueFile = fs::exists(basePath / "Value.bin");
        bool addressFile = fs::exists(basePath / "Address.bin");
        bool searchFile = fs::exists(basePath / "Search.txt");

        if(valueFile && addressFile && searchFile)
        {
            return true;
        }

        return false;
    }
}