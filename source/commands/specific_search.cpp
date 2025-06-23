#include "commands/specific_search.hpp"

#include <format>

namespace Command
{
    void SpecificSearch(Packet& packet)
    {
        // 例: 進捗を受け取るラムダ関数を登録
        Memory::SetProgressCallback([packet](float progress)
        {
            Packet response(packet.GetContext());
            response.Write<uint32_t>(std::to_underlying(Command::ID::SpecificSearch));
            response.Write<uint32_t>(std::to_underlying(Command::Response::Progress));
            response.Write<float>(progress);
            response.Send();
        });

        // packet から検索範囲や値を取得
        uint32_t start = packet.Read<uint32_t>();
        uint32_t size = packet.Read<uint32_t>();
        int value = packet.Read<int>();

        // サーチを実行
        Memory::Search(start, size, value);

        // 検索終了後にコールバックをクリア
        Memory::SetProgressCallback(nullptr);
    }
}