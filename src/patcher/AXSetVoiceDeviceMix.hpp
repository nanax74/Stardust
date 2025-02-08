#pragma once

#include <sndcore2/voice.h>
#include <sndcore2/device.h>

class AXSetVoiceDeviceMix
{
public:
    static void Start(AXVoice *voice, AXDeviceType type, uint32_t id, AXVoiceDeviceMixData *mixData);
    static void Update(AXVoice *voice, AXDeviceType type, uint32_t id, AXVoiceDeviceMixData *mixData);
    static bool GetIsStarted() {return isStarted;}
    static void SetIsStarted(bool value) {isStarted = value;}
    static void SwapMixDevice(AXDeviceType* type)
    {
        if(isSwap)
        {
            if(*type == AX_DEVICE_TYPE_TV)
            {
                *type = AX_DEVICE_TYPE_DRC;
            }
            else if(*type == AX_DEVICE_TYPE_DRC)
            {
                *type = AX_DEVICE_TYPE_TV;
            }
        }
    }
    static void SwapAudio()
    {
        isSwap = !isSwap;
    }

private:
    static bool isStarted;
    static bool isSwap;
};