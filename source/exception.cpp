#include "exception.hpp"

// std
#include <cstdio>
#include <string>
#include <cstring>
#include <format>

// wut
#include <coreinit/debug.h>
#include <whb/log.h>

// wums
#include "notification.hpp"

using namespace std;

namespace Exception
{
    // 通知用の変数
    vector<OSContext> contextBuffer;

    // クラッシュ処理
    void Panic(string exceptionType, OSContext* context)
    {
        string message = format("{0} Exception occurred from 0x{1:08X} in 0x{2:08X}", exceptionType, context->srr0, context->dar);
        OSFatal(message.c_str());
    }

    namespace SystemReset
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("SystemReset", context);
            return 0;
        }
    }

    namespace MachineCheck
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("MachineCheck", context);
            return 0;
        }
    }

    namespace DSI
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            // DABR による例外でなければクラッシュ
            if ((context->dsisr & (1 << 22)) == 0)
            {
                // メッセージを表示してクラッシュ
                Panic("DSI", context);
                return 0;
            }
            else
            {
                // 通知の準備
                contextBuffer.push_back(*context);

                // 復帰処理
                context->srr0 += 4;
                return 1;
            }
        }
    }

    namespace ISI
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("ISI", context);
            return 0;
        }
    }

    namespace ExternalInterrupt
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("ExternalInterrupt", context);
            return 0;
        }
    }

    namespace Alignment
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("Alignment", context);
            return 0;
        }
    }

    namespace Program
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("Program", context);
            return 0;
        }
    }

    namespace FloatingPoint
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("FloatingPoint", context);
            return 0;
        }
    }

    namespace Decrementer
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("Decrementer", context);
            return 0;
        }
    }

    namespace SystemCall
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("SystemCall", context);
            return 0;
        }
    }

    namespace Trace
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("Trace", context);
            return 0;
        }
    }

    namespace PerformanceMonitor
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("PerformanceMonitor", context);
            return 0;
        }
    }

    namespace Breakpoint
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("Breakpoint", context);
            return 0;
        }
    }

    namespace SystemInterrupt
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("SystemInterrupt", context);
            return 0;
        }
    }

    namespace ICI
    {
        int32_t ExceptionCallback(OSContext* context)
        {
            Panic("ICI", context);
            return 0;
        }
    }

    void SetExceptionCallbacks()
    {
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_SYSTEM_RESET, SystemReset::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_MACHINE_CHECK, MachineCheck::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_DSI, DSI::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_ISI, ISI::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_EXTERNAL_INTERRUPT, ExternalInterrupt::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_ALIGNMENT, Alignment::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_PROGRAM, Program::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_FLOATING_POINT, FloatingPoint::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_DECREMENTER, Decrementer::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_SYSTEM_CALL, SystemCall::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_TRACE, Trace::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_PERFORMANCE_MONITOR, PerformanceMonitor::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_BREAKPOINT, Breakpoint::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_SYSTEM_INTERRUPT, SystemInterrupt::ExceptionCallback);
        OSSetExceptionCallbackEx(OSExceptionMode::OS_EXCEPTION_MODE_GLOBAL_ALL_CORES, OSExceptionType::OS_EXCEPTION_TYPE_ICI, ICI::ExceptionCallback);
    }
}