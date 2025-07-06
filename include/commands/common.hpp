#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include <notifications/notifications.h>

#include "packet.hpp"

namespace Command
{
    // コマンドID
    enum class ID : uint32_t
    {
        // メモリ読み書き
        WriteMemory = 0x0,
        ReadMemory = 0x1,

        // メモリサーチ
        DumpForSearch = 0x10,
        SpecificSearch = 0x11,
        UnknownSearch = 0x12,
        ClearSearchFile = 0x13,
        CheckSearchFile = 0x14,

        // 応答要求(切断検出用)
        Respond = 0x1000,
        ReadMultipleMemory = 0x1001,

        // ダミー
        Dummy = 0xFFFFFFFF
    };

    // 応答
    enum class Response : uint32_t
    {
        Failure = 0x0,
        Success = 0x1,
        Progress = 0x2,
        Data = 0x3
    };

    enum class Comparison : uint32_t
    {
        Equal = 0,         // ==
        NotEqual = 1,      // !=
        LessThan = 2,      // <
        LessOrEqual = 3,   // <=
        GreaterThan = 4,   // >
        GreaterOrEqual = 5 // >=
    };

    enum class ValueType : uint32_t
    {
        Int8   = 0,
        UInt8  = 1,
        Int16  = 2,
        UInt16 = 3,
        Int32  = 4,
        UInt32 = 5,
        Int64  = 6,
        UInt64 = 7,
        Float  = 8,
        Double = 9,
        Unknown = 10
    };

    // 型からValueTypeクラスを取得
    template <typename T>
    ValueType GetValueType()
    {
        if constexpr(std::is_same_v<T, int8_t>) return ValueType::Int8;
        else if constexpr(std::is_same_v<T, uint8_t>) return ValueType::UInt8;
        else if constexpr(std::is_same_v<T, int16_t>) return ValueType::Int16;
        else if constexpr(std::is_same_v<T, uint16_t>) return ValueType::UInt16;
        else if constexpr(std::is_same_v<T, int32_t>) return ValueType::Int32;
        else if constexpr(std::is_same_v<T, uint32_t>) return ValueType::UInt32;
        else if constexpr(std::is_same_v<T, int64_t>) return ValueType::Int64;
        else if constexpr(std::is_same_v<T, uint64_t>) return ValueType::UInt64;
        else if constexpr(std::is_same_v<T, float>) return ValueType::Float;
        else if constexpr(std::is_same_v<T, double>) return ValueType::Double;
        else static_assert(sizeof(T) == 0, "Unsupported type for GetValueType");
    }

    // 値のサイズを取得
    size_t GetTypeSize(Command::ValueType type);

    // ValueTypeと文字列の変換関数
    std::string ValueTypeToString(Command::ValueType vt);
    Command::ValueType StringToValueType(const std::string& str);

    // ダミーハンドラ
    void Dummy(Packet& packet);
}

namespace Memory
{
    extern const std::filesystem::path STARDUST_PATH;

    using ProgressCallback = std::function<void(float progress)>;

    void SetProgressCallback(ProgressCallback callback);

    extern ProgressCallback g_ReportProgress;

    constexpr size_t KB = 1024;
    constexpr size_t MB = 1024 * KB;
    constexpr size_t GB = 1024 * MB;

    constexpr size_t THRESHOLD = 1 * MB;
    constexpr size_t BLOCK_SIZE = 1 * MB;

    // メタデータを保存
    bool Search_SaveMeta(uint32_t startAddress, uint32_t endAddress, Command::ValueType valueType);

    // メタデータを読み込み
    bool Search_LoadMeta(uint32_t& startAddress, uint32_t& endAddress, Command::ValueType& valueType);

    // アラインメント修正
    template <typename T>
    T AlignDown(T value, size_t alignment)
    {
        return value & ~(alignment - 1);
    }

    template <typename T>
    T AlignUp(T value, size_t alignment)
    {
        return (value + (alignment - 1)) & ~(alignment - 1);
    }

    template <typename T>
    void SendSearchResult(uint32_t address, T value, Command::ID cmd, std::shared_ptr<SocketContext>& context)
    {
        Packet response(context);
        response.Write(std::to_underlying(cmd));
        response.Write(std::to_underlying(Command::Response::Data));
        response.Write(address);
        response.Write(value);
        response.Send();
    }

    template <typename T>
    auto MakeSendSearchResultCallback(std::shared_ptr<SocketContext>& socket)
    {
        return [&socket](uint32_t address, T value, Command::ID cmd)
        {
            SendSearchResult<T>(address, value, cmd, socket);
        };
    }
}
