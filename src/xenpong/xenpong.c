#define INITGUID 1

#include <ntddk.h>
#include <wdm.h>

#include "xenpong.h"
#include "log.h"
#include "store_interface.h"

#define _DRIVER_NAME_ "XenPong"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#endif

NTSTATUS
DriverEntry(
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    Warning("Initializing DriverObject.\n");
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->DriverExtension->AddDevice = AddDevice;
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;

    return STATUS_SUCCESS;
}

VOID
DriverUnload(
    PDRIVER_OBJECT DriverObject
    )
{
}

NTSTATUS
AddDevice(
    PDRIVER_OBJECT DriverObject,
    PDEVICE_OBJECT pdo
    )
{
    PDEVICE_OBJECT DeviceObject;
    PDEVICE_EXTENSION pdx;
    NTSTATUS status;

    Warning("New device added.\n");
    status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION),
                            NULL, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN,
                            FALSE, &DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    pdx->DeviceObject = DeviceObject;
    pdx->Pdo = pdo;

    pdx->LowerDeviceObject = IoAttachDeviceToDeviceStack(DeviceObject, pdo);

    status = QueryStoreInterface(DeviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

NTSTATUS
DispatchPnp(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    PIO_STACK_LOCATION StackLocation;
    PDEVICE_EXTENSION pdx;
    UCHAR MinorFunction;
    NTSTATUS status;

    Warning("Received an IRP.\n");
    StackLocation = IoGetCurrentIrpStackLocation(Irp);
    MinorFunction = StackLocation->MinorFunction;
    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch (StackLocation->MinorFunction) {
        case IRP_MN_START_DEVICE:
            Warning("IRP_MN_START_DEVICE IRP received.\n");
            status = StartDevice(DeviceObject, Irp);
            break;
        case IRP_MN_STOP_DEVICE:
            Warning("IRP_MN_STOP_DEVICE IRP received.\n");
            status = StopDevice(DeviceObject, Irp);
            break;
        case IRP_MN_REMOVE_DEVICE:
            Warning("IRP_MN_REMOVE_DEVICE IRP received.\n");
            status = RemoveDevice(DeviceObject, Irp);
            break;
        default:
            Warning("IRP ID = %d.\n", StackLocation->MinorFunction);
            IoSkipCurrentIrpStackLocation(Irp);
            status = IoCallDriver(pdx->LowerDeviceObject, Irp);
            break;
    }

    return status;
}

_Function_class_(IO_COMPLETION_ROUTINE)
NTSTATUS
CompleteRoutine(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp,
    __in PVOID Context
    )
{
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
StartDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    PDEVICE_EXTENSION pdx;
    NTSTATUS status;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp, CompleteRoutine, NULL, TRUE, TRUE, TRUE);
    status = IoCallDriver(pdx->LowerDeviceObject, Irp);
    if (!NT_SUCCESS(status)) {
        Warning("IoCallDriver failed.\n");
    } else {
        Warning("IoCallDriver succeeded.\n");
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS
StopDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    PDEVICE_EXTENSION pdx;
    NTSTATUS status;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(pdx->LowerDeviceObject, Irp);

    return status;
}

NTSTATUS
RemoveDevice(
    __in PDEVICE_OBJECT DeviceObject,
    __in PIRP Irp
    )
{
    PDEVICE_EXTENSION pdx;
    NTSTATUS status;

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation(Irp);
    status = IoCallDriver(pdx->LowerDeviceObject, Irp);

    IoDetachDevice(pdx->LowerDeviceObject);
    IoDeleteDevice(DeviceObject);

    return status;
}

NTSTATUS
QueryStoreInterface(
    __in PDEVICE_OBJECT DeviceObject
    )
{
    INTERFACE Interface;
    IO_STATUS_BLOCK StatusBlock;
    KEVENT Event;
    PDEVICE_EXTENSION pdx;
    PIRP Irp;
    PIO_STACK_LOCATION StackLocation;
    NTSTATUS status;

    Warning("Querying Xenstore Interface.\n");

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    RtlZeroMemory(&StatusBlock, sizeof(IO_STATUS_BLOCK));

    pdx = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    Irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       pdx->LowerDeviceObject,
                                       NULL,
                                       0,
                                       NULL,
                                       &Event,
                                       &StatusBlock);

    if (Irp == NULL) {
        Warning("Unable to build synchronous FSD request.\n");
        return STATUS_UNSUCCESSFUL;
    }

    StackLocation = IoGetNextIrpStackLocation(Irp);
    StackLocation->MinorFunction = IRP_MN_QUERY_INTERFACE;

    StackLocation->Parameters.QueryInterface.InterfaceType = &GUID_STORE_INTERFACE;
    StackLocation->Parameters.QueryInterface.Size = sizeof(INTERFACE);
    StackLocation->Parameters.QueryInterface.Version = STORE_INTERFACE_VERSION;
    StackLocation->Parameters.QueryInterface.Interface = &Interface;

    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    status = IoCallDriver(pdx->LowerDeviceObject, Irp);
    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject(&Event,
                                     Executive,
                                     KernelMode,
                                     FALSE,
                                     NULL);
        status = StatusBlock.Status;
    }
    if (!NT_SUCCESS(status)) {
        Warning("Failed waiting on KEvent.\n");
        return status;
    }

    pdx->StoreInterface = Interface.Context;

    return STATUS_SUCCESS;
}
