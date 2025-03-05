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
        bool SaveMemoryList(const std::string& filePath, const MemoryList<T>& dist)
        {
            std::ofstream file(filePath, std::ios::binary);
            if (!file)
            {
                WHBLogPrintf("Failed to open file:%s", filePath.c_str());
                return false;
            }

            // 要素数を書き込む
            size_t size = dist.size();
            file.write(reinterpret_cast<const char*>(&size), sizeof(size));

            // メモリリストのデータを書き込む
            if(size > 0)
            {
                file.write(reinterpret_cast<const char*>(dist.data()), size * sizeof(MemoryEntry<T>));
            }

            file.close();
            return true;
        }

        // バイナリ形式でファイルから読み込む関数
        template <typename T>
        bool LoadMemoryList(const std::string& filePath, MemoryList<T>& src)
        {
            std::ifstream file(filePath, std::ios::binary);
            if (!file)
            {
                WHBLogPrintf("Failed to open file:%s", filePath.c_str());
                return false;
            }

            // 要素数を読み込む
            size_t size;
            file.read(reinterpret_cast<char*>(&size), sizeof(size));

            // メモリリストのデータを読み込む
            src.resize(size);
            if(size > 0)
            {
                file.read(reinterpret_cast<char*>(src.data()), size * sizeof(MemoryEntry<T>));
            }

            file.close();
            return true;
        }

        // 連番ファイルを削除
        void RemoveFileInOrder(std::string& dirPath, std::string fileName, std::string extension);

        bool FileExists(const std::string& filePath);

        // srcと現在のメモリをopで比較し、結果をdistに追加
        template <typename T>
        int SearchMemoryList(MemoryList<T>& src, MemoryList<T>& dist, CompareFlags op)
        {
            int count = 0;
            if(src.size() > 0)
            {
                for(MemoryEntry<T> item : src)
                {
                    CompareFlags cmp = Compare<T>(item.value, ReadMemory<T>(item.address));
                    if((cmp & op) == op)
                    {
                        dist.push_back(item);
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
            NotificationModuleHandle handleSub;
            NotificationModule_AddDynamicNotification("Cleaning Files...", &handleSub);
            base::RemoveFileInOrder(dirPath, fileName, "bin"); // ファイルをクリーンアップ
            
            NotificationModuleHandle handleMain;
            NotificationModule_AddDynamicNotification("Dumping...", &handleMain);

            unsigned int totalSize = size;
            unsigned int remaining = totalSize;
            unsigned int currentAddress = address;
            unsigned int part = 0;

            while (remaining > 0)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(part);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);

                unsigned int chunkSize = std::min(freeMemorySize, remaining);

                unsigned int endAddress = currentAddress + chunkSize;
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Reading:{:X}~{:X}", currentAddress, endAddress).c_str());
                WHBLogPrintf("Dumping:%X~%X", currentAddress, endAddress);
        
                MemoryList<T> list;
                base::GetMemoryList<T>(currentAddress, chunkSize, list);
                
                std::string distPath = std::format("{}/{}_{}.bin", dirPath, fileName, part);
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Saving:{}", distPath).c_str());
                base::SaveMemoryList<T>(distPath, list);
        
                remaining -= chunkSize;
                currentAddress += chunkSize;
                part++;
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

            int results = 0;
        
            unsigned int part = 0;
            while (true)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(part);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);
        
                std::string filePath = std::format("{}/{}_{}.bin", dirPath, fileName, part);

                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Checking file exists:{}", filePath).c_str());
                if(part > 0 && !base::FileExists(filePath))
                {
                    NotificationModule_AddInfoNotification("Dump complete!");
                    break;
                }
        
                MemoryList<T> src;
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Loading:{}", filePath).c_str());
                if(!base::LoadMemoryList<T>(filePath, src))
                {
                    NotificationModule_AddErrorNotification(std::format("Failed to load memory list from file:{}", filePath).c_str());
                    break;
                }
        
                MemoryList<T> dist;
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Searching:{}", filePath).c_str());
                results += base::SearchMemoryList<T>(src, dist, cmp);
        
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Saving:{}", filePath).c_str());
                if(!base::SaveMemoryList<T>(filePath, dist))
                {
                    NotificationModule_AddErrorNotification(std::format("Failed to save memory list to file:{}", filePath).c_str());
                    break;
                }
                part++;
            }
            NotificationModule_FinishDynamicNotification(handleSub, 0.1f);
            NotificationModule_FinishDynamicNotification(handleMain, 0.1f);
            return results;
        }
    }
}