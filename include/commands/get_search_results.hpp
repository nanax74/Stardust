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
    void GetSearchResults(size_t startIndex, size_t count, const std::function<void(uint32_t, T, Command::ID)>& onResult)
    {
        if (!onResult || count == 0) return;

        namespace fs = std::filesystem;
        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        const size_t valueSize = sizeof(T);
        const size_t addressSize = sizeof(uint32_t);

        std::ifstream valueFile(basePath / "Value.bin", std::ios::binary);
        std::ifstream addressFile(basePath / "Address.bin", std::ios::binary);

        if (!valueFile.is_open() || !addressFile.is_open())
        {
            throw std::runtime_error("Failed to open search files in DispatchResultsInRange");
        }

        // シーク
        valueFile.seekg(startIndex * valueSize, std::ios::beg);
        addressFile.seekg(startIndex * addressSize, std::ios::beg);

        std::vector<T> valueBuffer(count);
        std::vector<uint32_t> addressBuffer(count);

        valueFile.read(reinterpret_cast<char*>(valueBuffer.data()), count * valueSize);
        addressFile.read(reinterpret_cast<char*>(addressBuffer.data()), count * addressSize);

        for (size_t i = 0; i < count; ++i)
        {
            onResult(addressBuffer[i], valueBuffer[i], Command::ID::GetSearchResults);
        }
    }
}