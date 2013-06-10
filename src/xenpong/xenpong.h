#if !defined(_XENIFACE_H_)
#define _XENIFACE_H_

#include <ntddk.h>

#pragma warning(disable:4100 4057)

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_OBJECT LowerDeviceObject;
    PDEVICE_OBJECT Pdo;
    UNICODE_STRING ifname;
    IO_REMOVE_LOCK RemoveLock;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD DriverUnload;
DRIVER_ADD_DEVICE AddDevice;

_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH DispatchPnp;

NTSTATUS
StartDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    );

NTSTATUS
StopDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    );

NTSTATUS
RemoveDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    );

#endif  // _XENIFACE_H_
