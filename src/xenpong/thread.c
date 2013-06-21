#include <ntddk.h>
#include <wdm.h>

#include "thread.h"
#include "store.h"
#include "evtchn.h"
#include "log.h"
#include "util.h"
#include "xenpong.h"

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
    PXENPONG_THREAD StoreThread = Argument;
    PDEVICE_OBJECT DeviceObject = StoreThread->Context;
    PDEVICE_EXTENSION pdx;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Warning("StoreThread function.\n");

    Warning("Watching store for /vmping.\n");
    ReadChnFromStore(DeviceObject);

    Warning("RemoteId = %d\n", pdx->RemoteId);
    Warning("RemotePort = %d\n", pdx->RemotePort);

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
    PXENPONG_THREAD EvtchnThread = Argument;
    PDEVICE_OBJECT DeviceObject = EvtchnThread->Context;
    NTSTATUS status;

    Warning("Waiting for evtchn event to be signaled.\n");
    (VOID) KeWaitForSingleObject(&EvtchnThread->Event,
                                 Executive,
                                 KernelMode,
                                 FALSE,
                                 NULL);

    Warning("Evtchn event signaled.\n");
    status = ConnectEvtchnInterface(DeviceObject);
    if (NT_SUCCESS(status)) {
        Warning("Connected to event channel. Sending notification.\n");
        SendEvtchnNotify(DeviceObject);
    } else {
        Warning("Failed to connect to Event channel.\n");
    }

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
