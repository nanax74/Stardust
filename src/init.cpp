#include "main.hpp"

WUPS_PLUGIN_NAME("Stardust");
WUPS_PLUGIN_DESCRIPTION("Stardust");
WUPS_PLUGIN_VERSION("v1.0");
WUPS_PLUGIN_AUTHOR("Nanashi");
WUPS_PLUGIN_LICENSE("GPLv3");

INITIALIZE_PLUGIN()
{
    Main::Start();
}

DECL_FUNCTION(unsigned int, VPADRead, VPADChan chan, VPADStatus* buffer, unsigned int count, VPADReadError* error)
{
    Main::Update();
    return real_VPADRead(chan, buffer, count, error);
}

WUPS_MUST_REPLACE_FOR_PROCESS(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead, WUPS_FP_TARGET_PROCESS_ALL);