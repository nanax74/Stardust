#include "read_multiple_memory.hpp"

#include <vector>
#include <whb/log.h>
#include <coreinit/memorymap.h>

#include "commands/common.hpp"

namespace Command
{
    // 複数のメモリ領域を読み込むコマンドハンドラ
    void ReadMultipleMemory(Packet& packet)
    {
        CommandReceivedLogPrint("ReadMultipleMemory");

        // クライアントから読み取るメモリ領域の数を取得
        uint32_t numRegions = packet.Read<uint32_t>();

        // クライアントへ返信するレスポンスパケットを初期化
        Packet response(packet.GetContext());
        response.Write(static_cast<uint32_t>(Command::ID::ReadMultipleMemory)); // コマンドID
        response.Write(static_cast<uint32_t>(Command::Response::Data));         // 応答タイプ
        response.Write(numRegions);                                             // 読み取るメモリ領域の数

        // 各メモリ領域をループで処理
        for (uint32_t i = 0; i < numRegions; ++i)
        {
            uint32_t address = packet.Read<uint32_t>(); // メモリ領域の開始アドレスをパケットから読み込む
            uint32_t num_elements = packet.Read<uint32_t>(); // 読み取る要素の数
            Command::ValueType valueType = static_cast<Command::ValueType>(packet.Read<uint32_t>()); // 読み取る値の型をパケットから読み込む

            // 指定された値の型に応じた1要素あたりのバイトサイズを取得
            size_t element_size = Command::GetTypeSize(valueType);
            // 読み取るべき合計バイト数を計算 (要素数 * 1要素あたりのバイトサイズ)
            uint32_t total_byte_size = num_elements * element_size;

            // メモリ領域の有効性をチェック
            bool isValidRange = true;
            if (total_byte_size == 0 || address == 0 || !OSIsAddressValid(address) || !OSIsAddressValid(address + total_byte_size - 1))
            {
                isValidRange = false;
                WHBLogPrintf("ReadMultipleMemory: Invalid address or total_byte_size or range (address: 0x%X, num_elements: %u, valueType: %u, total_byte_size: %u)", address, num_elements, static_cast<uint32_t>(valueType), total_byte_size);
            }

            // レスポンスにアドレス、要素数、値の型を書き込む
            response.Write(address);
            response.Write(num_elements);
            response.Write(static_cast<uint32_t>(valueType));

            // メモリ領域が無効な場合
            if (!isValidRange)
            {
                // 無効なデータとして、合計バイト数分の0をレスポンスに書き込む
                for (uint32_t j = 0; j < total_byte_size; ++j)
                {
                    response.Write<uint8_t>(0);
                }
                continue; // 次のメモリ領域へ
            }

            // メモリをバイト単位で読み込み、レスポンスパケットに追加
            for (uint32_t j = 0; j < total_byte_size; ++j)
            {
                response.Write<uint8_t>(*reinterpret_cast<uint8_t*>(address + j));
            }
        }
        response.Send(); // 構築されたレスポンスパケットをクライアントに送信
        WHBLogPrintf("ReadMultipleMemory command finished.");
    }
}