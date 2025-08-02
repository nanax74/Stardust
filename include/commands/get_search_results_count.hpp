#pragma once

#include "commands/base.hpp"
#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

namespace Command
{
    class GetSearchResultsCount : public Base
    {
        public:
            GetSearchResultsCount() : Base("GetCurrentSearch") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    template <typename T>
    size_t GetSearchResultsCount()
    {
        namespace fs = std::filesystem;
        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        const size_t valueSize = sizeof(T);
        const size_t addressSize = sizeof(uint32_t);

        // ファイルのサイズを取得
        std::uintmax_t valueFileSize = fs::file_size(basePath / "Value.bin");
        std::uintmax_t addressFileSize = fs::file_size(basePath / "Address.bin");

        // エントリー数を取得
        const uint32_t entryCount = static_cast<uint32_t>(std::min(
            valueFileSize / valueSize,
            addressFileSize / addressSize
        ));

        return entryCount;
    }
}