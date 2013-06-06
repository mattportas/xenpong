#include "xenpong.h"
#include "log.h"

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
    Warning("Initializing DriverObject.");
    DriverObject->DriverUnload = DriverUnload;
    DriverObject->DriverExtension->AddDevice = AddDevice;

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
    PDEVICE_OBJECT fdo;
    PDEVICE_EXTENSION pdx;
    NTSTATUS status;

    status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &fdo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
    pdx->DeviceObject = fdo;
    pdx->Pdo = pdo;

    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}