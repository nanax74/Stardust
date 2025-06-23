#include "packet.hpp"

#include <cstring>
#include <stdexcept>
#include <type_traits>

#include <sys/socket.h>


Packet::Packet() : buffer(), cursor(0), socket(-1) {}

Packet::Packet(const std::vector<uint8_t>& data, int socket) : buffer(data), cursor(0), socket(socket) {}

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
    return socket;
}

void Packet::SetSocket(int s)
{
    socket = s;
}

void Packet::SendResponse(const std::vector<uint8_t>& response) const
{
    if (socket != -1)
    {
        ssize_t sent = send(socket, response.data(), response.size(), 0);
        if (sent < 0)
        {
            std::runtime_error("Failed to send response: " + std::string(strerror(errno)));
        }
    }
    else
    {
        throw std::runtime_error("No socket set for sending response");
    }
}