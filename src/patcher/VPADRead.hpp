#pragma once

#include <vpad/input.h>
#include "GX2CopyColorBufferToScanBuffer.hpp"

class VPADRead
{
public:
    static void Start(VPADStatus status);
    static void Update(VPADStatus status);
    static bool GetIsStarted() {return isStarted;}
    static void SetIsStarted(bool value) {isStarted = value;}

private:
    static bool isStarted;
};