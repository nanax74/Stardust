#include "packet.hpp"

#include <cstring>

Packet::Packet() : buffer(), cursor(0) {}

Packet::Packet(const std::vector<uint8_t>& data) : buffer(data), cursor(0) {}

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