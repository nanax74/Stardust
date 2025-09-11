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
#include "Stardust.hpp"
#include "logger.hpp"

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("TCPServer");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB(); // ファイルの読み書きに必須(無いとフリーズ)
WUPS_USE_STORAGE("Stardust"); // ストレージAPIの個別ID(なくても一応ファイルの読み書きはできるが念のため入れておく)

INITIALIZE_PLUGIN()
{
    Notification::Init();
    Exception::SetExceptionCallbacks();
    gStardust = std::make_unique<Stardust>();
}

ON_APPLICATION_START()
{
    Log::Init();
    if(!gStardust)
    {
        Log::Printf("Server::Create failed!");
        return;
    }
    gStardust->initialize();
}

ON_APPLICATION_REQUESTS_EXIT()
{
    if (gStardust)
    {
        gStardust->shutdown();
    }
    Log::Shutdown();
}