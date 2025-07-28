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
    }
}

ON_APPLICATION_REQUESTS_EXIT()
{
    if (gServer)
    {
        gServer.reset();
        gServer = nullptr;
    }
    Log::Shutdown();
}