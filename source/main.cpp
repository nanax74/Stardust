// std
#include <cstdint>

// wums
#include <notifications/notifications.h>

// wups
#include <wups.h>

// wut
#include <whb/log.h>
#include <whb/log_udp.h>

// stardust
#include "exception.hpp"
#include "server.hpp"

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("TCPServer");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB(); // ファイルの読み書きに必須(無いとフリーズ)
WUPS_USE_STORAGE("stardust"); // ストレージAPIの個別ID(なくても一応ファイルの読み書きはできるが念のため入れておく)

INITIALIZE_PLUGIN()
{
    NotificationModule_InitLibrary();
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 5.0f);
    Exception::SetExceptionCallbacks();
}

ON_APPLICATION_START()
{
    WHBLogUdpInit();
    gServer = Server::Create();
}

ON_APPLICATION_REQUESTS_EXIT()
{
    if (gServer)
    {
        gServer.reset();
        gServer = nullptr;
    }
    WHBLogUdpDeinit();
}

DEINITIALIZE_PLUGIN()
{
    if (gServer)
    {
        gServer.reset();
        gServer = nullptr;
    }
}