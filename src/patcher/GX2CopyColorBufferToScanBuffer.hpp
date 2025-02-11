#pragma once

#include <gx2/swap.h>
#include <gx2/surface.h>
#include <gx2/mem.h>
#include <gx2/draw.h>
#include <gx2/clear.h>
#include <gx2/registers.h>
#include <whb/gfx.h>

// libgui
//#include <gui/GuiText.h>
//#include <gui/video/CVideo.h>

#include "../class/color.hpp"
#include "../shader/shader.hpp"

class GX2CopyColorBufferToScanBuffer
{
public:
    static void Start(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget);
    static void Update(GX2ColorBuffer* colorBuffer, GX2ScanTarget scanTarget);
    static bool GetIsStarted() {return isStarted;}
    static void SetIsStarted(bool value) {isStarted = value;}
    static void SwapScanTarget(GX2ScanTarget* scanTarget)
    {
        if(isSwap)
        {
            if(*scanTarget == GX2_SCAN_TARGET_TV0)
            {
                *scanTarget = GX2_SCAN_TARGET_DRC0;
            }
            else if(*scanTarget == GX2_SCAN_TARGET_DRC0)
            {
                *scanTarget = GX2_SCAN_TARGET_TV0;
            }
        }
    }
    static void SwapScreen()
    {
        isSwap = !isSwap;
    }

private:
    static bool isStarted;
    static float hue;
    static bool isSwap;
};