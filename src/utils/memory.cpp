#include "memory.hpp"

// std
#include <cstdio>

using namespace std;

namespace memory
{
    namespace base
    {
        template <typename T>
        T ReadMemory(uint32_t address)
        {
            return *reinterpret_cast<T*>(address);
        }

        template int8_t ReadMemory<int8_t>(uint32_t);
        template uint8_t ReadMemory<uint8_t>(uint32_t);
        template int16_t ReadMemory<int16_t>(uint32_t);
        template uint16_t ReadMemory<uint16_t>(uint32_t);
        template int32_t ReadMemory<int32_t>(uint32_t);
        template uint32_t ReadMemory<uint32_t>(uint32_t);
        template int64_t ReadMemory<int64_t>(uint32_t);
        template uint64_t ReadMemory<uint64_t>(uint32_t);
        template float ReadMemory<float>(uint32_t);
        template double ReadMemory<double>(uint32_t);

        template <typename T>
        T ReadMemorySafe(uint32_t address)
        {
            if(OSIsAddressValid(address))
            {
                return *reinterpret_cast<T*>(address);
            }
            WHBLogPrintf("Address:%X is invalid", address);
            return 0;
        }

        template int8_t ReadMemorySafe<int8_t>(uint32_t);
        template uint8_t ReadMemorySafe<uint8_t>(uint32_t);
        template int16_t ReadMemorySafe<int16_t>(uint32_t);
        template uint16_t ReadMemorySafe<uint16_t>(uint32_t);
        template int32_t ReadMemorySafe<int32_t>(uint32_t);
        template uint32_t ReadMemorySafe<uint32_t>(uint32_t);
        template int64_t ReadMemorySafe<int64_t>(uint32_t);
        template uint64_t ReadMemorySafe<uint64_t>(uint32_t);
        template float ReadMemorySafe<float>(uint32_t);
        template double ReadMemorySafe<double>(uint32_t);

        template <typename T>
        void WriteMemory(uint32_t address, T value)
        {
            *reinterpret_cast<T*>(address) = value;
        }

        template void WriteMemory<int8_t>(uint32_t, int8_t);
        template void WriteMemory<uint8_t>(uint32_t, uint8_t);
        template void WriteMemory<int16_t>(uint32_t, int16_t);
        template void WriteMemory<uint16_t>(uint32_t, uint16_t);
        template void WriteMemory<int32_t>(uint32_t, int32_t);
        template void WriteMemory<uint32_t>(uint32_t, uint32_t);
        template void WriteMemory<int64_t>(uint32_t, int64_t);
        template void WriteMemory<uint64_t>(uint32_t, uint64_t);
        template void WriteMemory<float>(uint32_t, float);
        template void WriteMemory<double>(uint32_t, double);

        template <typename T>
        void WriteMemorySafe(uint32_t address, T value)
        {
            if(OSIsAddressValid(address))
            {
                *reinterpret_cast<T*>(address) = value;
            }
        }

        template void WriteMemorySafe<int8_t>(uint32_t, int8_t);
        template void WriteMemorySafe<uint8_t>(uint32_t, uint8_t);
        template void WriteMemorySafe<int16_t>(uint32_t, int16_t);
        template void WriteMemorySafe<uint16_t>(uint32_t, uint16_t);
        template void WriteMemorySafe<int32_t>(uint32_t, int32_t);
        template void WriteMemorySafe<uint32_t>(uint32_t, uint32_t);
        template void WriteMemorySafe<int64_t>(uint32_t, int64_t);
        template void WriteMemorySafe<uint64_t>(uint32_t, uint64_t);
        template void WriteMemorySafe<float>(uint32_t, float);
        template void WriteMemorySafe<double>(uint32_t, double);

        bool RemoveFile(std::string& filePath)
        {
            if (std::remove(filePath.c_str()) != 0)
            {
                WHBLogPrintf("File:%s does not exist", filePath.c_str());
                return false;
            }
            return true;
        }

        bool FileExists(const std::string& filePath)
        {
            std::ifstream file(filePath);
            return !file.fail();
        }

        // ファイル名変更関数
        bool RenameFile(const std::string& oldPath, const std::string& newPath)
        {
            if (std::rename(oldPath.c_str(), newPath.c_str()) != 0)
            {
                WHBLogPrintf("Error renaming file");  // エラーメッセージを表示
                return false;  // 失敗した場合
            }
            else
            {
                WHBLogPrintf("File renamed successfully!");  // 成功メッセージ
                return true;  // 成功した場合
            }
        }

        uint32_t GetFileSize(const std::string& filePath)
        {
            // ファイルをバイナリモードで開く
            std::ifstream file(filePath, std::ios::binary);
        
            // ファイルが開けなかった場合
            if (!file)
            {
                WHBLogPrintf("Invalid File Size:%s", filePath.c_str());
                return 0;
            }
        
            // ファイルの末尾までシーク
            file.seekg(0, std::ios::end);
        
            // 現在のポインタ位置（ファイルサイズ）を取得
            uint32_t fileSize = static_cast<uint32_t>(file.tellg());
        
            // ファイルを閉じる
            file.close();
        
            return fileSize;
        }

        template <typename T>
        bool SaveMemoryList(const std::string& distPath, const MemoryList<T>& src)
        {
            std::ofstream file(distPath, std::ios::app | std::ios::binary);
        
            file.write(reinterpret_cast<const char*>(src.data()), src.size() * sizeof(MemoryEntry<T>));
        
            file.close();
            return true;
        }

        template bool SaveMemoryList<int8_t>(const string&, const MemoryList<int8_t>&);
        template bool SaveMemoryList<uint8_t>(const string&, const MemoryList<uint8_t>&);
        template bool SaveMemoryList<int16_t>(const string&, const MemoryList<int16_t>&);
        template bool SaveMemoryList<uint16_t>(const string&, const MemoryList<uint16_t>&);
        template bool SaveMemoryList<int32_t>(const string&, const MemoryList<int32_t>&);
        template bool SaveMemoryList<uint32_t>(const string&, const MemoryList<uint32_t>&);
        template bool SaveMemoryList<int64_t>(const string&, const MemoryList<int64_t>&);
        template bool SaveMemoryList<uint64_t>(const string&, const MemoryList<uint64_t>&);
        template bool SaveMemoryList<float>(const string&, const MemoryList<float>&);
        template bool SaveMemoryList<double>(const string&, const MemoryList<double>&);

        template <typename T>
        size_t LoadMemoryList(const std::string& srcPath, MemoryList<T>& dist, uint32_t offset)
        {
            std::ifstream file(srcPath, std::ios::binary);
            if (!file)
            {
                WHBLogPrintf("Failed to open file:%s", srcPath.c_str());
                return 0;
            }
            
            size_t entrySize = sizeof(MemoryEntry<T>);
            
            uint32_t offsetSize = offset * entrySize;
            uint32_t readSize = GetFileSize(srcPath);

            if(readSize <= offsetSize)
            {
                WHBLogPrintf("Offset is out of range:%X", offset);
                return 0;
            }

            readSize = readSize - offsetSize;
            readSize = std::min(readSize, FREE_MEMORY_SIZE);  // 必要に応じて読み込むサイズを制限
            uint32_t readElements = readSize / entrySize;

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

        template size_t LoadMemoryList<int8_t>(const string&, MemoryList<int8_t>&, uint32_t);
        template size_t LoadMemoryList<uint8_t>(const string&, MemoryList<uint8_t>&, uint32_t);
        template size_t LoadMemoryList<int16_t>(const string&, MemoryList<int16_t>&, uint32_t);
        template size_t LoadMemoryList<uint16_t>(const string&, MemoryList<uint16_t>&, uint32_t);
        template size_t LoadMemoryList<int32_t>(const string&, MemoryList<int32_t>&, uint32_t);
        template size_t LoadMemoryList<uint32_t>(const string&, MemoryList<uint32_t>&, uint32_t);
        template size_t LoadMemoryList<int64_t>(const string&, MemoryList<int64_t>&, uint32_t);
        template size_t LoadMemoryList<uint64_t>(const string&, MemoryList<uint64_t>&, uint32_t);
        template size_t LoadMemoryList<float>(const string&, MemoryList<float>&, uint32_t);
        template size_t LoadMemoryList<double>(const string&, MemoryList<double>&, uint32_t);

        template <typename T>
        int32_t SearchMemoryList(MemoryList<T>& src, MemoryList<T>& dist, CompareFlags op)
        {
            int32_t count = 0;
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

        template int32_t SearchMemoryList<int8_t>(MemoryList<int8_t>&, MemoryList<int8_t>&, CompareFlags);
        template int32_t SearchMemoryList<uint8_t>(MemoryList<uint8_t>&, MemoryList<uint8_t>&, CompareFlags);
        template int32_t SearchMemoryList<int16_t>(MemoryList<int16_t>&, MemoryList<int16_t>&, CompareFlags);
        template int32_t SearchMemoryList<uint16_t>(MemoryList<uint16_t>&, MemoryList<uint16_t>&, CompareFlags);
        template int32_t SearchMemoryList<int32_t>(MemoryList<int32_t>&, MemoryList<int32_t>&, CompareFlags);
        template int32_t SearchMemoryList<uint32_t>(MemoryList<uint32_t>&, MemoryList<uint32_t>&, CompareFlags);
        template int32_t SearchMemoryList<int64_t>(MemoryList<int64_t>&, MemoryList<int64_t>&, CompareFlags);
        template int32_t SearchMemoryList<uint64_t>(MemoryList<uint64_t>&, MemoryList<uint64_t>&, CompareFlags);
        template int32_t SearchMemoryList<float>(MemoryList<float>&, MemoryList<float>&, CompareFlags);
        template int32_t SearchMemoryList<double>(MemoryList<double>&, MemoryList<double>&, CompareFlags);

        template <typename T>
        void GetMemoryList(uint32_t address, uint32_t size, MemoryList<T>& dist)
        {
            for(uint32_t i = 0; i < size; i += sizeof(T))
            {
                uint32_t currentAddress = address + i;
                dist.push_back({currentAddress, ReadMemory<T>(currentAddress)});
            }
        }

        template void GetMemoryList<int8_t>(uint32_t, uint32_t, MemoryList<int8_t>&);
        template void GetMemoryList<uint8_t>(uint32_t, uint32_t, MemoryList<uint8_t>&);
        template void GetMemoryList<int16_t>(uint32_t, uint32_t, MemoryList<int16_t>&);
        template void GetMemoryList<uint16_t>(uint32_t, uint32_t, MemoryList<uint16_t>&);
        template void GetMemoryList<int32_t>(uint32_t, uint32_t, MemoryList<int32_t>&);
        template void GetMemoryList<uint32_t>(uint32_t, uint32_t, MemoryList<uint32_t>&);
        template void GetMemoryList<int64_t>(uint32_t, uint32_t, MemoryList<int64_t>&);
        template void GetMemoryList<uint64_t>(uint32_t, uint32_t, MemoryList<uint64_t>&);
        template void GetMemoryList<float>(uint32_t, uint32_t, MemoryList<float>&);
        template void GetMemoryList<double>(uint32_t, uint32_t, MemoryList<double>&);
    }

    namespace advanced
    {
        template <typename T>
        void DumpMemory(int32_t socket, std::string& dirPath, std::string fileName, uint32_t address, uint32_t size)
        {
            std::string distPath = std::format("{}/{}.bin", dirPath, fileName);

            NotificationModuleHandle handleSub;
            NotificationModule_AddDynamicNotification("Cleaning Files...", &handleSub);
            base::RemoveFile(distPath); // ファイルをクリーンアップ
            
            NotificationModuleHandle handleMain;
            NotificationModule_AddDynamicNotification("Dumping...", &handleMain);
            
            uint32_t totalDump = 0;
            uint32_t remaining = size;
            uint32_t currentAddress = address;

            while (remaining > 0)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(totalDump);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);

                uint32_t chunkSize = std::min(FREE_MEMORY_SIZE, remaining);

                uint32_t endAddress = currentAddress + chunkSize;
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

        template void DumpMemory<int8_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<uint8_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<int16_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<uint16_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<int32_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<uint32_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<int64_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<uint64_t>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<float>(int32_t, string&, string, uint32_t, uint32_t);
        template void DumpMemory<double>(int32_t, string&, string, uint32_t, uint32_t);

        template <typename T>
        int32_t SearchMemory(int32_t socket, std::string& dirPath, std::string fileName, CompareFlags cmp)
        {
            NotificationModuleHandle handleSub;
            NotificationModule_AddDynamicNotification("Starting Search...", &handleSub);
        
            NotificationModuleHandle handleMain;
            NotificationModule_AddDynamicNotification("Searching...", &handleMain);
        
            std::string filePath = std::format("{}/{}.bin", dirPath, fileName);
            std::string tempPath = std::format("{}/temp.bin", dirPath);
        
            int32_t results = 0;
        
            uint32_t totalSearch = 0;
            while (true)
            {
                NotificationModule_UpdateDynamicNotificationText(handleSub, "Sending progress...");
                bytes progress;
                progress.append(totalSearch);
                data::SendPacket(socket, progress, DATA_TYPE_PROGRESS);
        
                MemoryList<T> src;
                WHBLogPrintf(std::format("Loading:{} Offset:{:X}", filePath, totalSearch).c_str());
                NotificationModule_UpdateDynamicNotificationText(handleSub, std::format("Loading:{} Offset:{:X}", filePath, totalSearch).c_str());
                uint32_t loaded = base::LoadMemoryList<T>(filePath, src, totalSearch);
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

        template int32_t SearchMemory<int8_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<uint8_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<int16_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<uint16_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<int32_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<uint32_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<int64_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<uint64_t>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<float>(int32_t, string&, string, CompareFlags);
        template int32_t SearchMemory<double>(int32_t, string&, string, CompareFlags);
    }
}