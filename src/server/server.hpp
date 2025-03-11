#pragma once

// std
#include <thread>

namespace TCPServer
{
    void Init();
    void Deinit();
    void Thread(std::stop_token token);
    int Start();
    void Update();
    void End();
    void AcceptClient();
    void DisconnectClient();
    void ReceiveFromClient();
}