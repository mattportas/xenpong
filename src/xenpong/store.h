#if !defined(_STORE_H_)
#define _STORE_H_

#include <ntddk.h>

NTSTATUS
ReadChnFromStore(
    __in PDEVICE_OBJECT DeviceObject
    );

#endif // _STORE_H_
