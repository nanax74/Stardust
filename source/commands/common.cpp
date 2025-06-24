#include "commands/common.hpp"

// std
#include <filesystem>
#include <format>
#include <fstream>
#include <regex>

namespace Command
{
    size_t GetTypeSize(Command::ValueType type)
    {
        switch (type)
        {
            case ValueType::Int8:   return sizeof(int8_t);
            case ValueType::UInt8:  return sizeof(uint8_t);
            case ValueType::Int16:  return sizeof(int16_t);
            case ValueType::UInt16: return sizeof(uint16_t);
            case ValueType::Int32:  return sizeof(int32_t);
            case ValueType::UInt32: return sizeof(uint32_t);
            case ValueType::Int64:  return sizeof(int64_t);
            case ValueType::UInt64: return sizeof(uint64_t);
            case ValueType::Float:  return sizeof(float);
            case ValueType::Double: return sizeof(double);
            default: throw std::invalid_argument("Unknown value type");
        }
    }

    // ValueTypeと文字列の変換関数
    std::string ValueTypeToString(Command::ValueType vt)
    {
        switch (vt)
        {
            case Command::ValueType::Int8:   return "Int8";
            case Command::ValueType::UInt8:  return "UInt8";
            case Command::ValueType::Int16:  return "Int16";
            case Command::ValueType::UInt16: return "UInt16";
            case Command::ValueType::Int32:  return "Int32";
            case Command::ValueType::UInt32: return "UInt32";
            case Command::ValueType::Int64:  return "Int64";
            case Command::ValueType::UInt64: return "UInt64";
            default:                        return "Unknown";
        }
    }

    bool StringToValueType(const std::string& str, Command::ValueType& outValueType)
    {
        if (str == "Int8")   { outValueType = Command::ValueType::Int8; return true; }
        if (str == "UInt8")  { outValueType = Command::ValueType::UInt8; return true; }
        if (str == "Int16")  { outValueType = Command::ValueType::Int16; return true; }
        if (str == "UInt16") { outValueType = Command::ValueType::UInt16; return true; }
        if (str == "Int32")  { outValueType = Command::ValueType::Int32; return true; }
        if (str == "UInt32") { outValueType = Command::ValueType::UInt32; return true; }
        if (str == "Int64")  { outValueType = Command::ValueType::Int64; return true; }
        if (str == "UInt64") { outValueType = Command::ValueType::UInt64; return true; }
        return false;
    }

    void Dummy(Packet& packet)
    {
        // ダミーのハンドラ。何もしない。
        Packet response(packet.GetContext());
        response.Write<uint32_t>(std::to_underlying(ID::Dummy));
        response.Write<uint32_t>(std::to_underlying(Response::Success)); // 成功応答
        response.Send();
    }
}

namespace Memory
{
    const std::filesystem::path STARDUST_PATH = "fs:/vol/external01/wiiu/Stardust";

    // コールバックの実体（ヘッダで extern されたもの）
    ProgressCallback g_ReportProgress = nullptr;

    void SetProgressCallback(ProgressCallback callback)
    {
        g_ReportProgress = callback;
    }

    // メタデータを保存
    bool Search_SaveMeta(uint32_t startAddress, uint32_t dumpSize, Command::ValueType valueType, bool compress)
    {
        std::ofstream ofs(STARDUST_PATH / "Search" / "Search.txt");
        if (!ofs.is_open()) return false;

        ofs << "startAddress=" << startAddress << "\n";
        ofs << "dumpSize=" << dumpSize << "\n";
        ofs << "valueType=" << static_cast<uint32_t>(valueType) << "\n";
        ofs << "compress=" << (compress ? "1" : "0") << "\n";

        ofs.close();
        return true;
    }

    // メタデータを読み込み
    bool Search_LoadMeta(uint32_t& startAddress, uint32_t& dumpSize, Command::ValueType& valueType, bool& compress)
    {
        std::ifstream ifs(STARDUST_PATH / "Search" / "Search.txt");
        if (!ifs.is_open()) return false;

        std::string line;
        while (std::getline(ifs, line))
        {
            auto delimPos = line.find('=');
            if (delimPos == std::string::npos) continue;

            std::string key = line.substr(0, delimPos);
            std::string value = line.substr(delimPos + 1);

            if (key == "startAddress")
                startAddress = static_cast<uint32_t>(std::stoul(value));
            else if (key == "dumpSize")
                dumpSize = static_cast<uint32_t>(std::stoul(value));
            else if (key == "valueType")
                valueType = static_cast<Command::ValueType>(std::stoul(value));
            else if (key == "compress")
                compress = (value == "1");
        }

        ifs.close();
        return true;
    }

    // 一括処理（圧縮データは返り値で返す）
    std::vector<uint8_t> CompressValues(const uint8_t* raw, size_t rawSize, size_t valueSize)
    {
        std::vector<uint8_t> output;

        if (valueSize == 0 || rawSize % valueSize != 0) return output;

        size_t count = rawSize / valueSize;
        size_t i = 0;

        while (i < count)
        {
            const uint8_t* currentValue = &raw[i * valueSize];
            size_t runLength = 1;

            // 最大255まで連続値をカウント
            while (i + runLength < count && runLength < 255)
            {
                const uint8_t* nextValue = &raw[(i + runLength) * valueSize];
                if (memcmp(currentValue, nextValue, valueSize) != 0) break;
                runLength++;
            }

            // 圧縮データを書き込み（1バイト回数 + 値サイズ分）
            uint8_t countByte = static_cast<uint8_t>(runLength);
            output.push_back(countByte);
            output.insert(output.end(), currentValue, currentValue + valueSize);

            i += runLength;
        }

        return output;
    }

    // ストリーミング処理（解凍データはコールバックで渡す）
    void DecompressValues(const uint8_t* compressed, size_t compressedSize, size_t valueSize, ValueCallback onDecompress)
    {
        size_t i = 0;
        while (i + 1 + valueSize <= compressedSize)
        {
            uint8_t count = compressed[i++];
            const uint8_t* value = &compressed[i];
            i += valueSize;

            for (uint8_t j = 0; j < count; ++j)
            {
                onDecompress(value);
            }
        }
    }

    // 一括処理（圧縮データは返り値で返す）
    std::vector<uint8_t> CompressAddresses(const std::vector<uint32_t>& addresses)
    {
        std::vector<uint8_t> output;
    
        if (addresses.empty()) return output;
    
        size_t i = 0;
        size_t n = addresses.size();
    
        while (i < n)
        {
            uint32_t baseAddr = addresses[i];
            uint32_t delta = 0;
            uint16_t repeatCount = 1;
        
            if (i + 1 < n)
            {
                delta = addresses[i + 1] - addresses[i];
            }
        
            if (delta > 0xFFFF || i + 1 >= n)
            {
                delta = 0;
                repeatCount = 1;
            }
            else
            {
                size_t j = i + 1;
                while (j < n && repeatCount < 0xFFFF)
                {
                    uint32_t currentDelta = addresses[j] - addresses[j - 1];
                    if (currentDelta != delta || currentDelta > 0xFFFF)
                    {
                        break;
                    }
                    ++repeatCount;
                    ++j;
                }
            }
        
            output.insert(
                output.end(),
                reinterpret_cast<uint8_t*>(&baseAddr),
                reinterpret_cast<uint8_t*>(&baseAddr) + sizeof(baseAddr)
            );
        
            uint16_t delta16 = static_cast<uint16_t>(delta);
            output.insert(
                output.end(),
                reinterpret_cast<uint8_t*>(&delta16),
                reinterpret_cast<uint8_t*>(&delta16) + sizeof(delta16)
            );
        
            output.insert(
                output.end(),
                reinterpret_cast<uint8_t*>(&repeatCount),
                reinterpret_cast<uint8_t*>(&repeatCount) + sizeof(repeatCount)
            );
        
            i += repeatCount;
        }
    
        return output;
    }

    // ストリーミング処理（解凍データはコールバックで渡す）
    void DecompressAddresses(const uint8_t* compressed, size_t compressedSize, AddressCallback onAddress)
    {
        size_t i = 0;
    
        while (i + 8 <= compressedSize)
        {
            // baseAddr（4バイト）
            const uint32_t* basePtr = reinterpret_cast<const uint32_t*>(&compressed[i]);
            uint32_t base = *basePtr;
        
            // delta（2バイト）
            const uint16_t* deltaPtr = reinterpret_cast<const uint16_t*>(&compressed[i + 4]);
            uint16_t delta = *deltaPtr;
        
            // repeatCount（2バイト）
            const uint16_t* repeatPtr = reinterpret_cast<const uint16_t*>(&compressed[i + 6]);
            uint16_t repeat = *repeatPtr;
        
            i += 8;
        
            for (uint16_t j = 0; j < repeat; ++j)
            {
                onAddress(base);
                base += delta;
            }
        }
    }
}