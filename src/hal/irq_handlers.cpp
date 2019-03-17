#include "error_handler.h"

extern "C"
void HardFault_Handler(void)
{
    getErrHndlr().halt(Libp::ErrCode::hard_fault);
}

extern "C"
void MemManage_Handler(void)
{
    getErrHndlr().halt(Libp::ErrCode::mem_manage);
}

extern "C"
void BusFault_Handler(void)
{
    getErrHndlr().halt(Libp::ErrCode::bus_fault);
}

extern "C"
void UsageFault_Handler(void)
{
    getErrHndlr().halt(Libp::ErrCode::usage_fault);
}


extern "C"
void NMI_Handler(void)
{
}

extern "C"
void SVC_Handler(void)
{
}

extern "C"
void DebugMon_Handler(void)
{
}

extern "C"
void PendSV_Handler(void)
{
}


// All exception are weak aliased to this handler in the startup file
/*extern "C"
void Default_Handler(void)
{
    getErrHndlr().halt(Libp::ErrCode::unhandled_exception, "unhandled exception");
}
*/
