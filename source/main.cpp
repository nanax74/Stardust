// std
#include <cstdint>

// wums
#include "notification.hpp"

// wups
#include <wups.h>

// wut
#include <coreinit/launch.h>
#include <coreinit/debug.h>

// stardust
#include "exception.hpp"
#include "server.hpp"
#include "logger.hpp"

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("TCPServer");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB(); // ファイルの読み書きに必須(無いとフリーズ)
WUPS_USE_STORAGE("stardust"); // ストレージAPIの個別ID(なくても一応ファイルの読み書きはできるが念のため入れておく)

#include "subscribed_address.hpp"

std::jthread g_subscription_thread;

INITIALIZE_PLUGIN()
{
    Notification::Init();
    Exception::SetExceptionCallbacks();
}

ON_APPLICATION_START()
{
    Log::Init();
    gServer = Server::Create();
    if(!gServer)
    {
        Log::Printf("Server::Create failed!");
        return;
    }

    g_subscription_thread = std::jthread([](std::stop_token stoken)
    {
        while (!stoken.stop_requested())
        {
            CheckSubscribedAddresses();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

ON_APPLICATION_REQUESTS_EXIT()
{
    if (g_subscription_thread.joinable()) {
        g_subscription_thread.request_stop();
        g_subscription_thread.join();
    }

    if (gServer)
    {
        gServer.reset();
        gServer = nullptr;
    }
    Log::Shutdown();
}