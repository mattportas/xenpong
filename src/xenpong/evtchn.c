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
