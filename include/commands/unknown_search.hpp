#pragma once

#include "commands/common.hpp"

// std
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>

// wut
#include <coreinit/memorymap.h>

// wums
#include "notification.hpp"

// stardust
#include "commands/base.hpp"
#include "packet.hpp"

namespace Command
{
    class UnknownSearch : public Base
    {
        public:
            UnknownSearch() : Base("UnknownSearch") {}
            void Process(Packet& packet) override;
    };
}

namespace Memory
{
    template <typename T>
    void UnknownSearch(uint32_t startAddress, uint32_t endAddress, Command::Comparison comparison)
    {
        // エイリアス
        using namespace std::chrono;
        namespace fs = std::filesystem;

        const size_t valueSize = sizeof(T);
        const size_t addressSize = sizeof(uint32_t);

        // アドレスがオーバーフローしていたらキャンセル
        if(startAddress >= endAddress)
        {
            throw std::runtime_error("End address is overflowing");
        }

        // アラインメント保証
        startAddress = AlignDown(startAddress, valueSize);
        endAddress = AlignUp(endAddress, valueSize);

        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        // ディレクトリの存在保証
        fs::create_directories(basePath);

        // 前回保存したファイルのストリーム
        std::ifstream valueFile(basePath / "Value.bin", std::ios::binary);
        std::ifstream addressFile(basePath / "Address.bin", std::ios::binary);
        if (!valueFile.is_open() || !addressFile.is_open())
        {
            throw std::runtime_error("Failed to open search files");
        }

        // ファイルのサイズを取得
        std::uintmax_t valueFileSize = fs::file_size(basePath / "Value.bin");
        std::uintmax_t addressFileSize = fs::file_size(basePath / "Address.bin");

        // エントリー数を取得
        const uint32_t entryCount = static_cast<uint32_t>(std::min(
            valueFileSize / valueSize,
            addressFileSize / addressSize
        ));

        // 保存するファイルのストリーム
        std::ofstream newValueFile(basePath / "Value.tmp", std::ios::binary);
        std::ofstream newAddressFile(basePath / "Address.tmp", std::ios::binary);
        if (!newValueFile.is_open() || !newAddressFile.is_open())
        {
            throw std::runtime_error("Failed to open temporary search files");
        }

        // バッファ
        std::vector<T> valueReadBuffer(THRESHOLD / valueSize);
        std::vector<uint32_t> addressReadBuffer(THRESHOLD / addressSize);

        std::vector<T> valueWriteBuffer;
        std::vector<uint32_t> addressWriteBuffer;

        // 進捗通知用のタイマー
        auto lastReport = std::chrono::steady_clock::now();

        // データ全体をブロック毎に処理
        for(uint32_t index = 0; index < entryCount;) // indexは現在の要素数
        {
            const uint32_t remaining = entryCount - index;
            const uint32_t entryCountInBlock = std::min(remaining, THRESHOLD / std::max(valueSize, addressSize)); // ブロック内の要素数

            // ファイル読み取り
            valueFile.read(reinterpret_cast<char*>(valueReadBuffer.data()), entryCountInBlock * valueSize);
            addressFile.read(reinterpret_cast<char*>(addressReadBuffer.data()), entryCountInBlock * addressSize);

            // ブロック全体を要素毎に処理
            for(uint32_t i = 0; i < entryCountInBlock; ++i, ++index)
            {
                const T oldValue = valueReadBuffer[i];
                const uint32_t address = addressReadBuffer[i];

                if(!OSIsAddressValid(address))
                {
                    continue;
                }

                // 値の読み取り
                T newValue = *reinterpret_cast<const T*>(address);

                if(Command::CompareValues(oldValue, newValue, comparison))
                {
                    (void)oldValue;
                    valueWriteBuffer.push_back(newValue);
                    addressWriteBuffer.push_back(address);
                }

                // 進捗報告
                auto now = std::chrono::steady_clock::now();
                if (now - lastReport >= std::chrono::milliseconds(100))
                {
                    float progress = static_cast<float>(index) / entryCount;
                    if (progress > 1.0f) progress = 1.0f; // 1.0fは超えないようにする
                    if (g_ReportProgress) g_ReportProgress(progress);
                    lastReport = now;
                }
            }

            // マッチした分だけ書き出す
            if (!valueWriteBuffer.empty() && !addressWriteBuffer.empty())
            {
                newValueFile.write(reinterpret_cast<const char*>(valueWriteBuffer.data()), valueWriteBuffer.size() * valueSize);
                newAddressFile.write(reinterpret_cast<const char*>(addressWriteBuffer.data()), addressWriteBuffer.size() * addressSize);

                valueWriteBuffer.clear();
                addressWriteBuffer.clear();
            }
        }

        valueFile.close();
        addressFile.close();
        newValueFile.close();
        newAddressFile.close();

        // 元ファイルを削除して上書き
        fs::remove(basePath / "Value.bin");
        fs::remove(basePath / "Address.bin");

        fs::rename(basePath / "Value.tmp", basePath / "Value.bin");
        fs::rename(basePath / "Address.tmp", basePath / "Address.bin");
    }
}
