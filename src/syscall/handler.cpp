#include "handler.hpp"

namespace Syscall
{
    uint32_t Handler(uint32_t id, uint32_t arg1)
    {
        switch (id)
        {
            case SET_DABR: SC_SetDABR(arg1); break;
        }
        return 0;
    }
}