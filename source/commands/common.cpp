#include "commands/common.hpp"

#include <filesystem>

namespace Command
{
    size_t GetValueSize(Command::ValueType type)
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
    ProgressCallback progressCallback = nullptr;

    void SetProgressCallback(ProgressCallback callback)
    {
        progressCallback = callback;
    }
}