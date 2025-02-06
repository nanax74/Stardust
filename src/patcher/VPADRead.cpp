#include "../main.hpp"
#include "VPADRead.hpp"

bool VPADRead::isStarted = false;

DECL_FUNCTION(unsigned int, VPADRead, VPADChan chan, VPADStatus* buffer, unsigned int count, VPADReadError* error)
{
    if(!VPADRead::GetIsStarted())
    {
        VPADRead::Start();
        VPADRead::SetIsStarted(true);
    }
    VPADRead::Update();
    return real_VPADRead(chan, buffer, count, error);
}

WUPS_MUST_REPLACE_FOR_PROCESS(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead, WUPS_FP_TARGET_PROCESS_ALL);

NotificationModuleHandle handle;

void VPADRead::Start()
{
    NotificationModule_AddDynamicNotificationEx("Text", &handle, {0, 0, 0, 255}, {255, 255, 255, 255}, nullptr, nullptr, false);
}

void VPADRead::Update()
{
    unsigned int value = *(unsigned int*)0x106E46E8;
    char str[9];
    sprintf(str, "%08X", value);
    NotificationModule_UpdateDynamicNotificationText(handle, str);
}