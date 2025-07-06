#pragma once

#include "commands/common.hpp"

// std
#include <filesystem>
#include <fstream>

// wut
#include <coreinit/memorymap.h>

// wums
#include <notifications/notifications.h>

// stardust
#include "packet.hpp"

namespace Command
{
    // メモリダンプコマンド
    void DumpForSearch(Packet& packet);
}

namespace Memory
{
    // メモリダンプ
    template <typename T>
    void Dump(uint32_t startAddress, uint32_t endAddress)
    {
        // アドレスがオーバーフローしていたらキャンセル
        if(startAddress >= endAddress)
        {
            throw std::runtime_error("End address is overflowing");
        }

        size_t valueSize = sizeof(T);

        // アラインメント保証
        startAddress = AlignDown(startAddress, valueSize);
        endAddress = AlignUp(endAddress, valueSize);

        uint32_t dumpSize = endAddress - startAddress;

        namespace fs = std::filesystem;

        fs::path basePath = fs::path(STARDUST_PATH) / "Search";

        // ディレクトリの存在保証
        fs::create_directories(basePath);

        // ファイルに書き込むためのストリーム
        std::ofstream valueFile(basePath / "Value.bin", std::ios::binary);
        std::ofstream addressFile(basePath / "Address.bin", std::ios::binary);
        if (!valueFile.is_open() || !addressFile.is_open())
        {
            throw std::runtime_error("Failed to open search files");
        }

        // バッファ
        std::vector<T> valueBuffer;
        std::vector<uint32_t> addressBuffer;

        // 進捗通知用のタイマー
        auto lastReport = std::chrono::steady_clock::now();

        // [アドレス読み取り->保存]のループ
        for (uint32_t address = startAddress; address < endAddress;)
        {
            // 読み取りと保存はブロック毎に行う(メモリ不足防止)
            uint32_t blockStartAddress = address;
            uint32_t blockEndAddress = std::min(address + BLOCK_SIZE, endAddress); // endAddressを超えないようにblockEndAddressを指定

            // アドレスの有効チェック
            if (!OSIsAddressValid(blockStartAddress) || !OSIsAddressValid(blockEndAddress - 1))
            {
                // このブロックはスキップ
                address = blockEndAddress;
                continue;
            }

            // ブロックを処理するループ
            for (; address < blockEndAddress; address += valueSize)
            {
                // 値の読み取り
                T value;
                std::memcpy(&value, reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), valueSize);

                // 値とアドレスをバッファに追加
                valueBuffer.push_back(value);
                addressBuffer.push_back(address);

                // バッファの中身が閾値を超えたら書き込み
                if (valueBuffer.size() >= THRESHOLD / valueSize)
                {
                    valueFile.write(reinterpret_cast<const char*>(valueBuffer.data()), valueBuffer.size() * valueSize);
                    valueBuffer.clear();
                }

                if (addressBuffer.size() >= THRESHOLD / sizeof(uint32_t))
                {
                    addressFile.write(reinterpret_cast<const char*>(addressBuffer.data()), addressBuffer.size() * sizeof(uint32_t));
                    addressBuffer.clear();
                }

                // 進捗報告
                auto now = std::chrono::steady_clock::now();
                if (now - lastReport >= std::chrono::milliseconds(100))
                {
                    float progress = static_cast<float>(address - startAddress) / dumpSize;
                    if (g_ReportProgress) g_ReportProgress(progress);
                    lastReport = now;
                }
            }
        }

        // 残りのバッファを書き込み
        if (!valueBuffer.empty())
        {
            valueFile.write(reinterpret_cast<const char*>(valueBuffer.data()), valueBuffer.size() * valueSize);
            valueFile.close();
            valueBuffer.clear();
        }

        if(!addressBuffer.empty())
        {
            addressFile.write(reinterpret_cast<const char*>(addressBuffer.data()), addressBuffer.size() * sizeof(uint32_t));
            addressFile.close();
            addressBuffer.clear();
        }

        if (g_ReportProgress) g_ReportProgress(1.0f);
    }
}