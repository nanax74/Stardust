#include "main.hpp"
#include "shaders/ColorShaderGX2.h"
#include "shaders/Texture2DShader.h"
#include "utils/logger.h"
// #include "retain_vars.hpp"
#include <coreinit/memory.h>

bool gOverlayInitDone;
GX2ContextState *gContextState;

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("Stardust");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

INITIALIZE_PLUGIN()
{
    initLogging();

    NotificationModule_InitLibrary();
    NotificationModule_SetDefaultValue(NOTIFICATION_MODULE_NOTIFICATION_TYPE_INFO, NOTIFICATION_MODULE_DEFAULT_OPTION_DURATION_BEFORE_FADE_OUT, 5.0f);
}

ON_APPLICATION_ENDS()
{
    ColorShader::destroyInstance();
    Texture2DShader::destroyInstance();
    deinitLogging();
}

DEINITIALIZE_PLUGIN()
{
    deinitLogging();
}