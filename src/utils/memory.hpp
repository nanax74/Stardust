#pragma once

// std
#include <algorithm>
#include <format>
#include <fstream>
#include <string>
#include <vector>

// wums
#include <notifications/notifications.h>

// wut
#include <coreinit/memorymap.h>
#include <whb/log.h>

// stardust
#include "../tcp/data.hpp"
#include "compare.hpp"

namespace memory
{
    const uint32_t FREE_MEMORY_SIZE = 0x100000;

    // アドレスと値のペア
    #pragma pack(push, 1) // アラインメントを1に設定
    template <typename T>
    struct MemoryEntry
    {
        uint32_t address;
        T value;
    };
    #pragma pack(pop)

    template <typename T>
    using MemoryList = std::vector<MemoryEntry<T>>;

    namespace base
    {
        // ファイルを削除
        bool RemoveFile(std::string& filePath);

        // ファイルの存在確認
        bool FileExists(const std::string& filePath);
        
        // ファイルの改名
        bool RenameFile(const std::string& oldPath, const std::string& newPath);
        
        // ファイルサイズを取得
        uint32_t GetFileSize(const std::string& filePath);

        template <typename T>
        T ReadMemory(uint32_t address);

        template <typename T>
        T ReadMemorySafe(uint32_t address);

        template <typename T>
        void WriteMemory(uint32_t address, T value);

        template <typename T>
        void WriteMemorySafe(uint32_t address, T value);

        void WriteKernel(uint32_t address, uint32_t value);

        // バイナリ形式でファイルに保存する関数
        template <typename T>
        bool SaveMemoryList(const std::string& distPath, const MemoryList<T>& src);

        // バイナリ形式でファイルから読み込む関数
        template <typename T>
        size_t LoadMemoryList(const std::string& srcPath, MemoryList<T>& dist, uint32_t offset);

        // srcと現在のメモリをopで比較し、結果をdistに追加
        template <typename T>
        int32_t SearchMemoryList(MemoryList<T>& src, MemoryList<T>& dist, CompareFlags op);

        // メモリを取得
        template <typename T>
        void GetMemoryList(uint32_t address, uint32_t size, MemoryList<T>& dist);
    }

    namespace advanced
    {
        template <typename T>
        void DumpMemory(int32_t socket, std::string& dirPath, std::string fileName, uint32_t address, uint32_t size);

        template <typename T>
        int32_t SearchMemory(int32_t socket, std::string& dirPath, std::string fileName, CompareFlags cmp);
    }
}