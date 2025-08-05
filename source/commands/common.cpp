#include "commands/common.hpp"
#include "logger.hpp"

// std
#include <filesystem>
#include <format>
#include <fstream>
#include <regex>
#include <string>

// wut
#include <whb/log.h>

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
            default:                         return "Unknown";
        }
    }

    Command::ValueType StringToValueType(const std::string& str)
    {
        if (str == "Int8")   { return Command::ValueType::Int8; }
        if (str == "UInt8")  { return Command::ValueType::UInt8; }
        if (str == "Int16")  { return Command::ValueType::Int16; }
        if (str == "UInt16") { return Command::ValueType::UInt16; }
        if (str == "Int32")  { return Command::ValueType::Int32; }
        if (str == "UInt32") { return Command::ValueType::UInt32; }
        if (str == "Int64")  { return Command::ValueType::Int64; }
        if (str == "UInt64") { return Command::ValueType::UInt64; }
        return Command::ValueType::Unknown;
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
    bool Search_SaveMeta(uint32_t startAddress, uint32_t dumpSize, Command::ValueType valueType)
    {
        std::ofstream ofs(STARDUST_PATH / "Search" / "Search.txt");
        if (!ofs.is_open()) return false;

        ofs << "StartAddress = " << startAddress << "\n";
        ofs << "EndAddress = " << dumpSize << "\n";
        ofs << "ValueType = " << static_cast<uint32_t>(valueType) << "\n";

        ofs.close();
        return true;
    }

    // メタデータを読み込み
    bool Search_LoadMeta(uint32_t& startAddress, uint32_t& endAddress, Command::ValueType& valueType)
    {
        std::ifstream ifs(STARDUST_PATH / "Search" / "Search.txt");
        if (!ifs.is_open()) return false;
    
        std::string line;
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\r\n"));
            s.erase(s.find_last_not_of(" \t\r\n") + 1);
        };
    
        while (std::getline(ifs, line))
        {
            auto delimPos = line.find('=');
            if (delimPos == std::string::npos) continue;
        
            std::string key = line.substr(0, delimPos);
            std::string value = line.substr(delimPos + 1);
            trim(key);
            trim(value);
        
            try
            {
                if (key == "StartAddress")
                    startAddress = static_cast<uint32_t>(std::stoul(value));
                else if (key == "EndAddress")
                    endAddress = static_cast<uint32_t>(std::stoul(value));
                else if (key == "ValueType")
                    valueType = static_cast<Command::ValueType>(std::stoul(value));
            }
            catch (...)
            {
                return false; // パース失敗
            }
        }
    
        return true;
    }
}