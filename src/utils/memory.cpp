#include "memory.hpp"

// std
#include <chrono>
#include <cstring>

// wut
#include <coreinit/memorymap.h>
#include <whb/log.h>

// stardust
#include "../tcp/data.hpp"
#include "math.hpp"

using namespace std;
using namespace data;

namespace memory
{
    unsigned char ReadUInt8(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned char*)address;
        }
        return 0;
    }
    
    unsigned short ReadUInt16(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned short*)address;
        }
        return 0;
    }
    
    unsigned int ReadUInt32(unsigned int address)
    {
        if(OSIsAddressValid(address))
        {
            return *(unsigned int*)address;
        }
        return 0;
    }

    void WriteUInt8(unsigned int address, unsigned char value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned char*)address = value;
        }
    }

    void WriteUInt16(unsigned int address, unsigned short value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned short*)address = value;
        }
    }

    void WriteUInt32(unsigned int address, unsigned int value)
    {
        if(OSIsAddressValid(address))
        {
            *(unsigned int*)address = value;
        }
    }

    // メモリをダンプしてファイルに保存
    bool MemoryDump(string dirPath, unsigned int startAddress, unsigned int size)
    {
        if(size == 0)
        {
            WHBLogPrintf("Dump size is zero");
            return false;
        }

        string destPath = dirPath + "/Dump.bin";
    
        RemoveFile(destPath); // ファイルが既に存在する場合は削除してから開始
    
        unsigned int totalDump = 0; // ダンプ済みサイズ
        while(totalDump < size)
        {
            unsigned int remainingSize = size - totalDump; // 残りサイズ
            unsigned int currentSize = min((unsigned int)BUFFER_SIZE, remainingSize); // このループでダンプするサイズ
            unsigned int currentStartAddress = startAddress + totalDump; // このループの開始アドレス
            unsigned int currentEndAddress = currentStartAddress + currentSize - 1; // このループの終了アドレス
            
            bytes buffer;
            buffer.resize(currentSize);
    
            if(OSIsAddressValid(currentStartAddress) && OSIsAddressValid(currentEndAddress))
            {
                memcpy(buffer.data(), reinterpret_cast<void*>(currentStartAddress), currentSize);
                WHBLogPrintf("Copied Data: %X~%X", currentStartAddress, currentEndAddress);
                WriteFile(destPath, buffer);
                WHBLogPrintf("Wrote Data: %X~%X", currentStartAddress, currentEndAddress);
                totalDump += currentSize;
            }
            else // 不正なアドレス
            {
                WHBLogPrintf("Invalid address is found");
                RemoveFile(destPath); // ファイルを削除して終了
                return false;
            }
        }
        return true;
    }

    // ダンプ結果からサーチ
    int SearchFromDump(string dirPath, unsigned int startAddress, unsigned int compareType)
    {
        unsigned int eachBufferSize = BUFFER_SIZE / 2; // バッファを2つ使用

        string srcPath = dirPath + "/Dump.bin";
        string distPath = dirPath + "/Search.bin";
        
        RemoveFile(distPath); // ファイルがある場合は削除してから開始

        unsigned int readSize = 0;
        GetFileSize(srcPath, readSize); // 読み取るファイルのサイズを取得
    
        unsigned int totalRead = 0;

        bytes bufferDump;
        bufferDump.resize(eachBufferSize); // ダンプファイルの中身を格納するバッファを事前確保
    
        while(totalRead < readSize)
        {
            unsigned int currentStartAddress = startAddress + totalRead;
            unsigned int remainingSize = readSize - totalRead;
            unsigned int readChunkSize = min(eachBufferSize, remainingSize);
            
            bytes bufferSearch; // サーチ結果を格納するバッファ

            if(OSIsAddressValid(currentStartAddress) && OSIsAddressValid(currentStartAddress + readChunkSize - 1))
            {
                ReadFile(srcPath, bufferDump, totalRead, readChunkSize); // ダンプ読み取り

                for(unsigned int i = 0; i < readChunkSize; i += 4)
                {
                    unsigned int currentAddress = currentStartAddress + i;
                    unsigned int currentValue = bufferDump.getUInt(i);
                    if(Compare(*(unsigned int*)currentAddress, currentValue, compareType))
                    {
                        bufferSearch.append(currentAddress);
                        bufferSearch.append(currentValue);
                    }
                }
                WriteFile(distPath, bufferSearch);

                totalRead += readChunkSize;
            }
            else // 不正アドレスがあれば終了
            {
                WHBLogPrintf("Invalid address is found");
                RemoveFile(distPath); // ファイルを削除して終了
                return 0;
            }
        }
        RemoveFile(distPath);
        return readSize;
    }

    int SearchFromSearch(string dirPath, unsigned int startAddress, unsigned int compareType)
    {
        unsigned int eachBufferSize = BUFFER_SIZE / 2; // バッファを2つ使用

        string searchPath = dirPath + "/Search.bin"; // ダンプ結果が保存されているファイル
        string tempPath = dirPath + "/temp.bin"; // サーチ結果を保存するファイル
        
        RemoveFile(tempPath); // ファイルがある場合は削除してから開始

        unsigned int readSize = 0;
        GetFileSize(searchPath, readSize); // 読み取るファイルのサイズを取得
    
        unsigned int totalRead = 0;
    
        unsigned int results = 0;
        while(totalRead < readSize)
        {
            unsigned int currentStartAddress = startAddress + totalRead;
            unsigned int remainingSize = readSize - totalRead;
            unsigned int readChunkSize = min((int)eachBufferSize, (int)remainingSize);

            bytes bufferSearch;
            bufferSearch.resize(readChunkSize); // ダンプファイルの中身を格納するバッファを事前確保
            
            bytes bufferTemp; // サーチ結果を格納するバッファ

            if(OSIsAddressValid(currentStartAddress) && OSIsAddressValid(currentStartAddress + readChunkSize - 1))
            {
                ReadFile(searchPath, bufferSearch, totalRead, readChunkSize); // 前のサーチ結果を読み取る

                for(unsigned int i = 0; i < readChunkSize; i += 8)
                {
                    unsigned int address = bufferSearch.getUInt(i); // Search.binからアドレスを取得

                    unsigned int currentValue = *(unsigned int*)address; // 現在の値
                    unsigned int previousValue = bufferSearch.getUInt(i + 4); // 前回のサーチ時の値

                    if(Compare(currentValue, previousValue, compareType))
                    {
                        bufferTemp.append(address);
                        bufferTemp.append(currentValue);
                        results++;
                    }
                }
                WriteFile(tempPath, bufferTemp);

                totalRead += readChunkSize;
            }
            else
            {
                RemoveFile(tempPath); // ファイルを削除して終了
                return 0; // 不正アドレスがあれば終了
            }
        }
        RemoveFile(searchPath); // 既存のSearch.binは削除
        RenameFile(tempPath, searchPath); // temp.bin -> Search.bin
        WHBLogPrintf("Results:%d", results);
        return results;
    }
}