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

void Packet::Send() const
{
    if (!ctx || !ctx->connected)
    {
        throw std::runtime_error("No valid socket context for sending response");
    }

    std::lock_guard<std::mutex> lock(ctx->sendMutex);

    ssize_t sent = send(ctx->socket, buffer.data(), buffer.size(), 0);
    if (sent < 0)
    {
        throw std::runtime_error("Failed to send response: " + std::string(strerror(errno)));
    }

    if ((size_t)sent != buffer.size())
    {
        // 送信できなかった分がある場合の処理（例: 再送要求など）
        throw std::runtime_error("Partial send occurred");
    }
}