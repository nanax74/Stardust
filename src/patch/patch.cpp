#include "patch.hpp"

// wums
#include <kernel/kernel.h>

// wut
#include <coreinit/exception.h>

// stardust
#include "../syscall/handler.hpp"
#include "../exception/callback.hpp"

using namespace Syscall;
using namespace Exception;

void PatchKernel()
{
    KernelPatchSyscall(0x6D, reinterpret_cast<uint32_t>(&Handler));
    SetExceptionCallbacks();
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