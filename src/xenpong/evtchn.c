#include <ntddk.h>
#include <wdm.h>

#include "evtchn.h"
#include "evtchn_interface.h"
#include "log.h"
#include "xenpong.h"

_Function_class_(KSERVICE_ROUTINE)
BOOLEAN
EvtchnCallback(
    __in PKINTERRUPT InterruptObject,
    __in PVOID Argument
    )
{
    PDEVICE_EXTENSION pdx = Argument;

    Warning("Event Callback.\n");

    return NT_SUCCESS(SendEvtchnNotify(pdx->DeviceObject));
}

NTSTATUS
ConnectEvtchnInterface(
    __in PDEVICE_OBJECT DeviceObject
    )
{
    BOOLEAN pending;
    PDEVICE_EXTENSION pdx;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    Warning("Acquring event channel.\n");
    EVTCHN(Acquire, pdx->EvtchnInterface);

    Warning("Opening event channel.\n");
    pdx->Evtchn = EVTCHN(Open,
                         pdx->EvtchnInterface,
                         EVTCHN_INTER_DOMAIN,
                         EvtchnCallback,
                         pdx,
                         pdx->RemoteId,
                         pdx->RemotePort,
                         FALSE);

    if (pdx->Evtchn == NULL) {
        Warning("Failed to open eventchannel.\n");
        EVTCHN(Release, pdx->EvtchnInterface);
        return STATUS_UNSUCCESSFUL;
    }

    Warning("Unmasking event channel.\n");
    pending = EVTCHN(Unmask,
                     pdx->EvtchnInterface,
                     pdx->Evtchn,
                     FALSE);

    if (pending) {
        Warning("Handling pending event trigger.\n");
        EVTCHN(Trigger,
               pdx->EvtchnInterface,
               pdx->Evtchn);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
DisconnectEvtchnInterface(
    __in PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION pdx;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Warning("Closing event channel.\n");
    EVTCHN(Close,
           pdx->EvtchnInterface,
           pdx->Evtchn);
    pdx->Evtchn = NULL;

    EVTCHN(Release, pdx->EvtchnInterface);
    pdx->EvtchnInterface = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS
SendEvtchnNotify(
    __in PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION pdx;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    (VOID) EVTCHN(Send,
                  pdx->EvtchnInterface,
                  pdx->Evtchn);

    return STATUS_SUCCESS;
}
