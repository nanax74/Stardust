#include "memory.hpp"

// std
#include <cstdio>

using namespace std;

namespace memory
{
    namespace base
    {
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

        unsigned int GetFileSize(const std::string& filePath)
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
            unsigned int fileSize = static_cast<unsigned int>(file.tellg());
        
            // ファイルを閉じる
            file.close();
        
            return fileSize;
        }
    }
}