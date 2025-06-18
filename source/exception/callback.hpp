#pragma once

// std
#include <cstdint>
#include <vector>

// wut
#include <coreinit/exception.h>

namespace Exception
{
    extern std::vector<OSContext> contextBuffer;

    namespace SystemReset
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace MachineCheck
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace DSI
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace ISI
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace ExternalInterrupt
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace Alignment
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace Program
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace FloatingPoint
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace Decrementer
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace SystemCall
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace Trace
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace PerformanceMonitor
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace Breakpoint
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace SystemInterrupt
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    namespace ICI
    {
        int32_t ExceptionCallback(OSContext* context);
    }

    void SetExceptionCallbacks();
}