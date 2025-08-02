#pragma once

#include "commands/base.hpp"
#include "commands/common.hpp"

#include <filesystem>
#include <fstream>

namespace Command
{
    class GetSearchResults : public Base
    {
        public:
            GetSearchResults() : Base("GetCurrentSearch") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    // startIndex: 取得するサーチ結果の開始インデックス
    // count: 取得するサーチ結果の数
    template <typename T>
    void GetSearchResults(const std::function<void(uint32_t, T, Command::ID)>& onResult)
    {
        if (!onResult) return;

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

        std::ifstream valueFile(basePath / "Value.bin", std::ios::binary);
        std::ifstream addressFile(basePath / "Address.bin", std::ios::binary);

        if (!valueFile.is_open() || !addressFile.is_open())
        {
            throw std::runtime_error("Failed to open search files in DispatchResultsInRange");
        }

        std::vector<T> valueBuffer(entryCount);
        std::vector<uint32_t> addressBuffer(entryCount);

        valueFile.read(reinterpret_cast<char*>(valueBuffer.data()), entryCount * valueSize);
        addressFile.read(reinterpret_cast<char*>(addressBuffer.data()), entryCount * addressSize);

        for (size_t i = 0; i < entryCount; ++i)
        {
            onResult(addressBuffer[i], valueBuffer[i], Command::ID::GetSearchResults);
        }
    }
}