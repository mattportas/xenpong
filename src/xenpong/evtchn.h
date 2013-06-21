#if !defined(_EVTCHN_H_)
#define _EVTCHN_H_

#include <ntddk.h>
#include <wdm.h>

NTSTATUS
ConnectEvtchnInterface(
    __in PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DisconnectEvtchnInterface(
    __in PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SendEvtchnNotify(
    __in PDEVICE_OBJECT DeviceObject
    );

#endif // _EVTCHN_H_
