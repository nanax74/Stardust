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

        // メモリダンプ
        DumpMemory = 0x10,

        // メモリサーチ
        SpecificSearch = 0x20,
        UnknownSearch = 0x21,

        // ダミー
        Dummy = 0xFFFFFFFF
    };

    // 応答
    enum class Response : uint32_t
    {
        Success = 0x0,
        Failure = 0x1,
        Progress = 0x2
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
        Double = 9
    };

    // 値のサイズを取得
    size_t GetTypeSize(Command::ValueType type);

    // ValueTypeと文字列の変換関数
    std::string ValueTypeToString(Command::ValueType vt);
    bool StringToValueType(const std::string& str, Command::ValueType& outValueType);

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
    bool Search_SaveMeta(uint32_t startAddress, uint32_t dumpSize, Command::ValueType valueType, bool compress);

    // メタデータを読み込み
    bool Search_LoadMeta(uint32_t& startAddress, uint32_t& dumpSize, Command::ValueType& valueType, bool& compress);

    using ValueCallback = std::function<void(const uint8_t* value)>;
    using AddressCallback = std::function<void(uint32_t address)>;

    // 一括処理（圧縮データは返り値で返す）
    std::vector<uint8_t> CompressValues(const uint8_t* raw, size_t rawSize, size_t valueSize);

    // ストリーミング処理（解凍データはコールバックで渡す）
    void DecompressValues(const uint8_t* compressed, size_t compressedSize, size_t valueSize, ValueCallback onDecompress);

    // 一括処理（圧縮データは返り値で返す）
    std::vector<uint8_t> CompressAddresses(const std::vector<uint32_t>& addresses);

    // ストリーミング処理（解凍データはコールバックで渡す）
    void DecompressAddresses(const uint8_t* compressed, size_t compressedSize, AddressCallback onAddress);
}
