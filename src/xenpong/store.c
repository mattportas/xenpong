#include <ntddk.h>
#include <wdm.h>
#include <stdlib.h>

#include "log.h"
#include "store.h"
#include "store_interface.h"
#include "xenpong.h"

NTSTATUS
ReadChnFromStore(
    __in PDEVICE_OBJECT DeviceObject
    )
{
    KTIMER Timer;
    LARGE_INTEGER timeout = {0};
    PDEVICE_EXTENSION pdx;
    PCHAR Buffer;
    ULONG DomId;
    ULONG Port;
    NTSTATUS status;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    KeInitializeTimerEx(&Timer, SynchronizationTimer);
    KeSetTimerEx(&Timer, timeout, 500, NULL);

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Warning("Reading from Store.\n");
    for (;;) {

        (VOID) KeWaitForSingleObject(&Timer,
                                     Executive,
                                     KernelMode,
                                     FALSE,
                                     NULL);

        STORE(Acquire, pdx->StoreInterface);

        status = STORE(Read,
                       pdx->StoreInterface,
                       NULL,
                       "vmping",
                       "domid",
                       &Buffer);

        if (!NT_SUCCESS(status)) {
            STORE(Release, pdx->StoreInterface);
            continue;
        }

        Warning("Read vmping/domid.\n");
        DomId = strtoul(Buffer, NULL, 10);
        STORE(Free, pdx->StoreInterface, Buffer);

        status = STORE(Read,
                       pdx->StoreInterface,
                       NULL,
                       "vmping",
                       "port",
                       &Buffer);
        if (!NT_SUCCESS(status)) {
            STORE(Release, pdx->StoreInterface);
            continue;
        }

        Warning("Read vmping/port.\n");
        Port = strtoul(Buffer, NULL, 10);
        STORE(Free, pdx->StoreInterface, Buffer);

        KeCancelTimer(&Timer);

        STORE(Release, pdx->StoreInterface);
        break;
    }

    pdx->RemoteId = DomId;
    pdx->RemotePort = Port;

    return STATUS_SUCCESS;
}
