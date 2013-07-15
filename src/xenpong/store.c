/* Copyright (c) 2013 Citrix Systems Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 * * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <ntddk.h>
#include <wdm.h>
#include <stdlib.h>

#include "log.h"
#include "thread.h"
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
    PXENPONG_THREAD EvtchnThread;
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

        EvtchnThread = pdx->EvtchnThread;
        KeSetEvent(&EvtchnThread->Event, IO_NO_INCREMENT, FALSE);

        break;
    }

    pdx->RemoteId = DomId;
    pdx->RemotePort = Port;

    return STATUS_SUCCESS;
}
