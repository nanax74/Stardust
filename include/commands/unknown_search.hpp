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
#include <notifications/notifications.h>

// stardust
#include "packet.hpp"

namespace Command
{
    // 未知の値を検索するコマンド
    void UnknownSearch(Packet& packet);
}

namespace Memory
{
    template <typename T>
    void UnknownSearch(uint32_t startAddress, uint32_t endAddress, Command::ValueType valueType, Command::Comparison comparison, std::function<void(uint32_t, T, Command::ID)> onResult = nullptr)
    {
        // アドレスがオーバーフローしていたらキャンセル
        if(startAddress >= endAddress)
        {
            throw std::runtime_error("End address is overflowing");
        }

        const size_t valueSize = Command::GetTypeSize(valueType);
        const size_t addressSize = sizeof(uint32_t);

        // アラインメント保証
        startAddress = AlignDown(startAddress, valueSize);
        endAddress = AlignUp(endAddress, valueSize);

        namespace fs = std::filesystem;

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

        std::uintmax_t valueFileSize = fs::file_size(basePath / "Value.bin");
        std::uintmax_t addressFileSize = fs::file_size(basePath / "Address.bin");

        // 最小のエントリ数で揃える
        const uint32_t entryCount = static_cast<uint32_t>(std::min(
            valueFileSize / valueSize,
            addressFileSize / addressSize
        ));

        // 今回保存するファイルのストリーム
        std::ofstream newValueFile(basePath / "Value.tmp", std::ios::binary);
        std::ofstream newAddressFile(basePath / "Address.tmp", std::ios::binary);
        if (!newValueFile.is_open() || !newAddressFile.is_open())
        {
            throw std::runtime_error("Failed to open temporary search files");
        }

        // バッファ
        std::vector<T> valueReadBuffer;
        valueReadBuffer.resize(THRESHOLD / valueSize);
        std::vector<T> valueWriteBuffer;

        std::vector<uint32_t> addressReadBuffer;
        addressReadBuffer.resize(THRESHOLD / addressSize);
        std::vector<uint32_t> addressWriteBuffer;

        // 進捗通知用のタイマー
        auto lastReport = std::chrono::steady_clock::now();

        // 結果が何件かを格納
        uint32_t matchCount = 0;

        // データ全体をブロック毎に処理
        for(uint32_t index = 0; index < entryCount;) // indexは現在の要素数
        {
            const uint32_t remaining = entryCount - index;
            const uint32_t entryCountInBlock = std::min(remaining, THRESHOLD / std::max(valueSize, addressSize)); // ブロック内の要素数

            // ファイル読み取り
            valueFile.read(reinterpret_cast<char*>(valueReadBuffer.data()), valueSize * entryCountInBlock);
            addressFile.read(reinterpret_cast<char*>(addressReadBuffer.data()), addressSize * entryCountInBlock);

            // ブロック全体を要素毎に処理
            for(uint32_t i = 0; i < entryCountInBlock; i++, index++)
            {
                const T oldValue = valueReadBuffer[i];
                const uint32_t address = addressReadBuffer[i];

                if(!OSIsAddressValid(address))
                {
                    continue;
                }

                // 値の読み取り
                T newValue;
                std::memcpy(&newValue, reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), valueSize);

                bool match = false;
                switch (comparison)
                {
                    case Command::Comparison::Equal:         match = (newValue == oldValue); break;
                    case Command::Comparison::NotEqual:      match = (newValue != oldValue); break;
                    case Command::Comparison::LessThan:      match = (newValue < oldValue); break;
                    case Command::Comparison::LessOrEqual:   match = (newValue <= oldValue); break;
                    case Command::Comparison::GreaterThan:   match = (newValue > oldValue); break;
                    case Command::Comparison::GreaterOrEqual:match = (newValue >= oldValue); break;
                    default: throw std::runtime_error("Unknown comparison type");
                }

                if(match)
                {
                    matchCount++;
                    valueWriteBuffer.push_back(newValue);
                    addressWriteBuffer.push_back(address);
                }

                // 進捗報告
                auto now = std::chrono::steady_clock::now();
                if (now - lastReport >= std::chrono::milliseconds(100))
                {
                    float progress = static_cast<float>(index) / entryCount;

                    // 1.0fには達しないようにする
                    if (progress >= 1.0f) progress = 0.999f;

                    if (g_ReportProgress) g_ReportProgress(progress);

                    lastReport = now;
                }
            }

            newValueFile.write(reinterpret_cast<const char*>(valueWriteBuffer.data()), valueWriteBuffer.size() * valueSize);
            valueWriteBuffer.clear();

            newAddressFile.write(reinterpret_cast<const char*>(addressWriteBuffer.data()), addressWriteBuffer.size() * addressSize);
            addressWriteBuffer.clear();
        }

        valueFile.close();
        addressFile.close();
        newValueFile.close();
        newAddressFile.close();

        fs::remove(basePath / "Value.bin");
        fs::remove(basePath / "Address.bin");

        fs::rename(basePath / "Value.tmp", basePath / "Value.bin");
        fs::rename(basePath / "Address.tmp", basePath / "Address.bin");

        if (onResult && matchCount < 10000)
        {
            std::ifstream sendValueFile(basePath / "Value.bin", std::ios::binary);
            std::ifstream sendAddressFile(basePath / "Address.bin", std::ios::binary);

            for (uint32_t i = 0; i < matchCount; ++i)
            {
                T value;
                uint32_t address;

                sendValueFile.read(reinterpret_cast<char*>(&value), sizeof(T));
                sendAddressFile.read(reinterpret_cast<char*>(&address), sizeof(uint32_t));

                onResult(address, value, Command::ID::UnknownSearch);
            }

            sendValueFile.close();
            sendAddressFile.close();
        }

        if (g_ReportProgress) g_ReportProgress(1.0f);
    }
}
