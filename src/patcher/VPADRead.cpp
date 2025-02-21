#include "VPADRead.hpp"

DECL_FUNCTION(unsigned int, VPADRead, VPADChan chan, VPADStatus* status, unsigned int count, VPADReadError* error)
{
    static bool isStarted = false;
    if(!isStarted)
    {
        VPAD::Start(*status);
        isStarted = true;
    }
    VPAD::Update(*status);
    return real_VPADRead(chan, status, count, error);
}

WUPS_MUST_REPLACE_FOR_PROCESS(VPADRead, WUPS_LOADER_LIBRARY_VPAD, VPADRead, WUPS_FP_TARGET_PROCESS_ALL);

namespace VPAD
{
    void Start(VPADStatus status)
    {
        TCPServer::Start();
    }
    
    void Update(VPADStatus status)
    {
        TCPServer::Update();
    }
}