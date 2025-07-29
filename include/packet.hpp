#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <type_traits>

#include "socket_context.hpp"

class Packet
{
public:
    Packet(std::shared_ptr<SocketContext> ctx = nullptr);
    Packet(const std::vector<uint8_t>& data, std::shared_ptr<SocketContext> ctx = nullptr);

    template <typename T>
    void Write(const T& value);

    template <typename T>
    T Read();

    void WriteString(const std::string& str);
    std::string ReadString();

    const std::vector<uint8_t>& GetData() const;
    void ResetCursor();
    size_t Size();

    int GetSocket() const;
    std::shared_ptr<SocketContext> GetContext() const;
    static std::vector<Packet> ExtractPackets(std::vector<uint8_t>& buffer, std::shared_ptr<SocketContext> ctx);
    void Send() const;

    std::string ToString() const;

private:
    std::vector<uint8_t> buffer;
    size_t cursor = 0;
    std::shared_ptr<SocketContext> ctx;
};

// テンプレート関数の実装はヘッダー末尾に
template <typename T>
void Packet::Write(const T& value)
{
    static_assert(std::is_trivially_copyable<T>::value, "Write<T>: unsupported type");
    const uint8_t* data = reinterpret_cast<const uint8_t*>(&value);
    buffer.insert(buffer.end(), data, data + sizeof(T));
}

template <typename T>
T Packet::Read()
{
    static_assert(std::is_trivially_copyable<T>::value, "Read<T>: unsupported type");
    if (cursor + sizeof(T) > buffer.size())
    {
        throw std::out_of_range("Read<T>: buffer overflow");
    }
    T value;
    std::memcpy(&value, buffer.data() + cursor, sizeof(T));
    cursor += sizeof(T);
    return value;
}
