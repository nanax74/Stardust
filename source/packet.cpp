#include "packet.hpp"

#include <cstring>
#include <stdexcept>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include <sys/socket.h>

#include "socket_context.hpp"

Packet::Packet(std::shared_ptr<SocketContext> ctx)
    : ctx(ctx) {}

Packet::Packet(const std::vector<uint8_t>& data, std::shared_ptr<SocketContext> ctx)
    : buffer(data), ctx(ctx) {}

void Packet::WriteString(const std::string& str)
{
    Write<uint32_t>(static_cast<uint32_t>(str.size()));
    buffer.insert(buffer.end(), str.begin(), str.end());
}

std::string Packet::ReadString()
{
    uint32_t length = Read<uint32_t>();
    if (cursor + length > buffer.size())
    {
        throw std::out_of_range("ReadString: buffer overflow");
    }
    std::string result(buffer.begin() + cursor, buffer.begin() + cursor + length);
    cursor += length;
    return result;
}

const std::vector<uint8_t>& Packet::GetData() const
{
    return buffer;
}

void Packet::ResetCursor()
{
    cursor = 0;
}

size_t Packet::Size()
{
    return buffer.size();
}

int Packet::GetSocket() const
{
    return ctx ? ctx->socket : -1;
}

std::shared_ptr<SocketContext> Packet::GetContext() const
{
    return ctx;
}

// バイト列から Packet を複数抽出
std::vector<Packet> Packet::ReceivePackets(std::vector<uint8_t>& buffer, std::shared_ptr<SocketContext> ctx)
{
    std::vector<Packet> packets;

    while (buffer.size() >= 4)
    {
        uint32_t packetSize;
        std::memcpy(&packetSize, buffer.data(), 4);

        if (buffer.size() < 4 + packetSize)
            break;

        std::vector<uint8_t> data(buffer.begin() + 4, buffer.begin() + 4 + packetSize);
        packets.emplace_back(data, ctx);

        buffer.erase(buffer.begin(), buffer.begin() + 4 + packetSize);
    }

    return packets;
}

void Packet::Send() const
{
    if (!ctx || !ctx->connected)
    {
        throw std::runtime_error("No valid socket context for sending response");
    }

    std::lock_guard<std::mutex> lock(ctx->sendMutex);

    uint32_t size = static_cast<uint32_t>(buffer.size());
    std::vector<uint8_t> fullMessage;

    // 長さ（4バイト）を先頭に追加（ネットワークバイトオーダーに変換しておくのが理想）
    fullMessage.insert(fullMessage.end(),
                       reinterpret_cast<uint8_t*>(&size),
                       reinterpret_cast<uint8_t*>(&size) + sizeof(uint32_t));

    // 本体を追加
    fullMessage.insert(fullMessage.end(), buffer.begin(), buffer.end());

    ssize_t sent = send(ctx->socket, fullMessage.data(), fullMessage.size(), 0);
    if (sent < 0)
    {
        throw std::runtime_error("Failed to send response: " + std::string(strerror(errno)));
    }
    if ((size_t)sent != fullMessage.size())
    {
        throw std::runtime_error("Partial send occurred");
    }
}
