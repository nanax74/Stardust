#include "commands/check_search_file.hpp"
#include "commands/common.hpp"

#include <filesystem>

namespace Command
{
    void CheckSearchFile::Process(Packet& packet)
    {
        Bool isExists = Memory::CheckSearchFile() == true ? Bool::True : Bool::False;
        Packet response(packet.GetContext());
        response.Write(std::to_underlying(ID::CheckSearchFile));
        response.Write(std::to_underlying(isExists));
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