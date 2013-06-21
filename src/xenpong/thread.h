#if !defined(_THREAD_H_)
#define _THREAD_H_

#include <ntddk.h>

typedef struct _XENPONG_THREAD {
    PVOID Context;
    KEVENT Event;
    PKTHREAD Thread;
} XENPONG_THREAD, *PXENPONG_THREAD;

_Function_class_(KSTART_ROUTINE)
VOID
StoreThreadFunction(
    __in PVOID Argument
    );

NTSTATUS
StoreThreadCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __out PXENPONG_THREAD *Thread
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
