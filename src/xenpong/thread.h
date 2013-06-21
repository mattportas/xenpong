#if !defined(_THREAD_H_)
#define _THREAD_H_

#include <ntddk.h>


_Function_class_(KSTART_ROUTINE)
VOID
StoreThreadFunction(
    __in PVOID Argument
    );

NTSTATUS
StoreThreadCreate(
    __in PDEVICE_OBJECT DeviceObject
    );

_Function_class_(KSTART_ROUTINE)
VOID
EvtchnThreadFunction(
    __in PVOID Argument
    );

NTSTATUS
EvtchnThreadCreate(
    );

#endif // _THREAD_H_
