#include "main.hpp"

// wums
#include <notifications/notifications.h>

// wups
#include <wups.h>

// wut
#include <coreinit/exception.h>
#include <whb/log.h>
#include <whb/log_udp.h>

// stardust
#include "server/server.hpp"
#include "patch/patch.hpp"

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("TCPServer");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB(); // ファイルの読み書きに必須(無いとフリーズ)
WUPS_USE_STORAGE("stardust"); // ストレージAPIの個別ID(なくても一応ファイルの読み書きはできるが念のため入れておく)

TCPServer* server;

INITIALIZE_PLUGIN()
{
    NotificationModule_InitLibrary();
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 5.0f);
    PatchKernel();
}

ON_APPLICATION_START()
{
    WHBLogUdpInit();
    server = new TCPServer(9999);
}

ON_APPLICATION_REQUESTS_EXIT()
{
    if (server)
    {
        delete server;
        server = nullptr;
    }
    WHBLogUdpDeinit();
}