#include <ntddk.h>
#include <wdm.h>

#include "thread.h"
#include "log.h"
#include "util.h"

#define THREAD_POOL 'ERHT'

static FORCEINLINE PVOID
__ThreadAlloacte(
    __in ULONG Length
    )
{
    return __AllocateNonPagedPoolWithTag(Length, THREAD_POOL);
}

_Function_class_(KSTART_ROUTINE)
VOID
StoreThreadFunction(
    __in PVOID Argument
    )
{
    UNREFERENCED_PARAMETER(Argument);

    Warning("StoreThread function.\n");

    PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS
StoreThreadCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __out PXENPONG_THREAD *StoreThread
    )
{
    HANDLE Handle;
    NTSTATUS status;

    (*StoreThread) = __ThreadAlloacte(sizeof (XENPONG_THREAD));
    (*StoreThread)->Context = DeviceObject;

    KeInitializeEvent(&(*StoreThread)->Event, NotificationEvent, FALSE);

    status = PsCreateSystemThread(&Handle,
                                  STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  StoreThreadFunction,
                                  *StoreThread);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ObReferenceObjectByHandle(Handle,
                                       SYNCHRONIZE,
                                       *PsThreadType,
                                       KernelMode,
                                       &(*StoreThread)->Thread,
                                       NULL);

    ZwClose(Handle);

    return status;
}

_Function_class_(KSTART_ROUTINE)
VOID
EvtchnThreadFunction(
    __in PVOID Argument
    )
{
    UNREFERENCED_PARAMETER(Argument);
    PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS
EvtchnThreadCreate(
    __in PDEVICE_OBJECT DeviceObject,
    __out PXENPONG_THREAD *EvtchnThread
    )
{
    HANDLE Handle;
    NTSTATUS status;

    (*EvtchnThread) = __ThreadAlloacte(sizeof (XENPONG_THREAD));
    (*EvtchnThread)->Context = DeviceObject;

    KeInitializeEvent(&(*EvtchnThread)->Event, NotificationEvent, FALSE);

    status = PsCreateSystemThread(&Handle,
                                  STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  EvtchnThreadFunction,
                                  *EvtchnThread);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ObReferenceObjectByHandle(Handle,
                                       SYNCHRONIZE,
                                       *PsThreadType,
                                       KernelMode,
                                       &(*EvtchnThread)->Thread,
                                       NULL);

    ZwClose(Handle);

    return status;
}
