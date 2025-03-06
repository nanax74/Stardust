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
    const unsigned int freeMemorySize = 0x100000;

    // アドレスと値のペア
    #pragma pack(push, 1) // アラインメントを1に設定
    template <typename T>
    struct MemoryEntry
    {
        unsigned int address;
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
        unsigned int GetFileSize(const std::string& filePath);

        template <typename T>
        T ReadMemory(unsigned int address)
        {
            return *reinterpret_cast<T*>(address);
        }

        template <typename T>
        T ReadMemorySafe(unsigned int address)
        {
            if(OSIsAddressValid(address))
            {
                return *reinterpret_cast<T*>(address);
            }
            WHBLogPrintf("Address:%X is invalid", address);
            return 0;
        }

        template <typename T>
        void WriteMemory(unsigned int address, T value)
        {
            *reinterpret_cast<T*>(address) = value;
        }

        template <typename T>
        void WriteMemorySafe(unsigned int address, T value)
        {
            if(OSIsAddressValid(address))
            {
                *reinterpret_cast<T*>(address) = value;
            }
        }

        // バイナリ形式でファイルに保存する関数
        template <typename T>
        bool SaveMemoryList(const std::string& distPath, const MemoryList<T>& src)
        {
            std::ofstream file(distPath, std::ios::app | std::ios::binary);
        
            file.write(reinterpret_cast<const char*>(src.data()), src.size() * sizeof(MemoryEntry<T>));
        
            file.close();
            return true;
        }        

        // バイナリ形式でファイルから読み込む関数
        template <typename T>
        size_t LoadMemoryList(const std::string& srcPath, MemoryList<T>& dist, unsigned int offset)
        {
            std::ifstream file(srcPath, std::ios::binary);
            if (!file)
            {
                WHBLogPrintf("Failed to open file:%s", srcPath.c_str());
                return 0;
            }
            
            size_t entrySize = sizeof(MemoryEntry<T>);
            
            unsigned int offsetSize = offset * entrySize;
            unsigned int readSize = GetFileSize(srcPath);

            if(readSize <= offsetSize)
            {
                WHBLogPrintf("Offset is out of range:%X", offset);
                return 0;
            }

            readSize = readSize - offsetSize;
            readSize = std::min(readSize, freeMemorySize);  // 必要に応じて読み込むサイズを制限
            unsigned int readElements = readSize / entrySize;

            // メモリリストのデータを読み込む
            dist.resize(readElements);
            if (readSize > 0)
            {
                file.seekg(offsetSize, std::ios::beg); // ファイルの先頭からオフセット位置にシーク
                file.read(reinterpret_cast<char*>(dist.data()), readSize);
            }
            
            file.close();
            return readElements;
        }

        // srcと現在のメモリをopで比較し、結果をdistに追加
        template <typename T>
        int SearchMemoryList(MemoryList<T>& src, MemoryList<T>& dist, CompareFlags op)
        {
            int count = 0;
            if(src.size() > 0)
            {
                for(MemoryEntry<T> item : src)
                {
                    T newValue = ReadMemory<T>(item.address);
                    CompareFlags cmp = Compare<T>(newValue, item.value);
                    if((cmp & op) == op)
                    {
                        dist.push_back({item.address, newValue});
                        ++count;
                    }
                }
            }
            return count;
        }

        // メモリを取得
        template <typename T>
        void GetMemoryList(unsigned int address, unsigned int size, MemoryList<T>& dist)
        {
            for(unsigned int i = 0; i < size; i += sizeof(T))
            {
                unsigned int currentAddress = address + i;
                dist.push_back({currentAddress, ReadMemory<T>(currentAddress)});
            }
        }
    }

    namespace advanced
    {
        template <typename T>
        void DumpMemory(int socket, std::string& dirPath, std::string fileName, unsigned int address, unsigned int size)
        {
            std::string distPath = std::format("{}/{}.bin", dirPath, fileName);

            NotificationModuleHandle handleSub;
            NotificationModule_AddDynamicNotification("Cleaning Files...", &handleSub);
            base::RemoveFile(distPath); // ファイルをクリーンアップ
            
            NotificationModuleHandle handleMain;
            NotificationModule_AddDynamicNotification("Dumping...", &handleMain);
            
            unsigned int totalDump = 0;
            unsigned int remaining = size;
            unsigned int currentAddress = address;

            while (remaining > 0)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(totalDump);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);

                unsigned int chunkSize = std::min(freeMemorySize, remaining);

                unsigned int endAddress = currentAddress + chunkSize;
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Reading:{:X}~{:X}", currentAddress, endAddress).c_str());
                WHBLogPrintf("Dumping:%X~%X", currentAddress, endAddress);
        
                MemoryList<T> list;
                base::GetMemoryList<T>(currentAddress, chunkSize, list);
                
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Saving:{}", distPath).c_str());
                base::SaveMemoryList<T>(distPath, list);
        
                remaining -= chunkSize;
                totalDump += chunkSize;
                currentAddress += chunkSize;
            }
            NotificationModule_FinishDynamicNotification(handleSub, 0.1f);
            NotificationModule_FinishDynamicNotification(handleMain, 0.1f);
        }

        template <typename T>
        int SearchMemory(int socket, std::string& dirPath, std::string fileName, CompareFlags cmp)
        {
            NotificationModuleHandle handleSub;
            NotificationModule_AddDynamicNotification("Starting Search...", &handleSub);
        
            NotificationModuleHandle handleMain;
            NotificationModule_AddDynamicNotification("Searching...", &handleMain);
        
            std::string filePath = std::format("{}/{}.bin", dirPath, fileName);
            std::string tempPath = std::format("{}/temp.bin", dirPath);
        
            int results = 0;
        
            unsigned int totalSearch = 0;
            while (true)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(totalSearch);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);
        
                MemoryList<T> src;
                WHBLogPrintf(std::format("Loading:{} Offset:{:X}", filePath, totalSearch).c_str());
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Loading:{} Offset:{:X}", filePath, totalSearch).c_str());
                unsigned int loaded = base::LoadMemoryList<T>(filePath, src, totalSearch);
                if(loaded == 0)
                {
                    WHBLogPrintf(std::format("Failed to load memory list from file:{}", filePath).c_str());
                    break;
                }
        
                MemoryList<T> dist;
                WHBLogPrintf(std::format("Searching:{}bytes", src.size()).c_str());
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Searching:{}bytes", src.size()).c_str());
                results += base::SearchMemoryList<T>(src, dist, cmp);
        
                WHBLogPrintf(std::format("Saving:{} {}bytes", tempPath, dist.size()).c_str());
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Saving:{} {}bytes", tempPath, dist.size()).c_str());
                if(!base::SaveMemoryList<T>(tempPath, dist))
                {
                    WHBLogPrintf(std::format("Failed to save memory list to file:{}", filePath).c_str());
                    break;
                }
                totalSearch += loaded;
        
                // デバッグ用ログを追加
                WHBLogPrintf("Total search progress: %u", totalSearch);
                WHBLogPrintf("Current results count: %d", results);
            }
            WHBLogPrintf("Broke");
            base::RemoveFile(filePath);
            base::RenameFile(tempPath, filePath);
            NotificationModule_FinishDynamicNotification(handleSub, 0.1f);
            NotificationModule_FinishDynamicNotification(handleMain, 0.1f);
            return results;
        }
    }
}