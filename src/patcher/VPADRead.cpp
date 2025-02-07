#include "../main.hpp"
#include "VPADRead.hpp"

bool VPADRead::isStarted = false;

DECL_FUNCTION(unsigned int, VPADRead, VPADChan chan, VPADStatus* status, unsigned int count, VPADReadError* error)
{
    if(!VPADRead::GetIsStarted())
    {
        VPADRead::Start(*status);
        VPADRead::SetIsStarted(true);
    }
    VPADRead::Update(*status);
    return real_VPADRead(chan, status, count, error);
}

WUPS_MUST_REPLACE_FOR_PROCESS(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead, WUPS_FP_TARGET_PROCESS_ALL);

NotificationModuleHandle handle;

void VPADRead::Start(VPADStatus status)
{
    NotificationModule_AddDynamicNotificationEx("Text", &handle, {0, 0, 0, 255}, {255, 255, 255, 255}, nullptr, nullptr, false);
}

void VPADRead::Update(VPADStatus status)
{
    if(status.trigger & VPAD_BUTTON_A)
    {
        GX2CopyColorBufferToScanBuffer::SwapScreen();
    }

    unsigned int value = *(unsigned int*)0x106E46E8;
    char str[9];
    sprintf(str, "%08X", value);
    NotificationModule_UpdateDynamicNotificationText(handle, str);
}