#include "memory.hpp"

using namespace std;

namespace memory
{
    namespace base
    {
        void RemoveFileInOrder(std::string& dirPath, std::string fileName, std::string extension)
        {
            int index = 0;
            while (true)
            {
                std::string filePath = std::format("{}/{}_{}.{}", dirPath, fileName, index, extension);
                if (std::remove(filePath.c_str()) != 0)
                {
                    WHBLogPrintf("File:%s does not exist", filePath.c_str());
                    // ファイルが存在しない場合、終了
                    break;
                }
                index++;
            }
            WHBLogPrintf("No more files to delete:%s", fileName.c_str());
        }
    }
}