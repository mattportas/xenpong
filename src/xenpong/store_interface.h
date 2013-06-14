//
// Copyright 2011 Citrix Systems Inc. All rights reserved.
// Use is subject to license terms.
//

#ifndef _XENBUS_STORE_INTERFACE_H
#define _XENBUS_STORE_INTERFACE_H

typedef struct _XENBUS_STORE_TRANSACTION    XENBUS_STORE_TRANSACTION, *PXENBUS_STORE_TRANSACTION;
typedef struct _XENBUS_STORE_WATCH          XENBUS_STORE_WATCH, *PXENBUS_STORE_WATCH;

#define DEFINE_STORE_OPERATIONS                                                 \
        STORE_OPERATION(VOID,                                                   \
                        Acquire,                                                \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context                 \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(VOID,                                                   \
                        Release,                                                \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context                 \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(VOID,                                                   \
                        Free,                                                   \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PCHAR                       Value                   \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Read,                                                   \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction OPTIONAL,   \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node,                   \
                        OUT PCHAR                       *Value                  \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Write,                                                  \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction OPTIONAL,   \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node,                   \
                        IN  PCHAR                       Value                   \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Printf,                                                 \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction OPTIONAL,   \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node,                   \
                        IN  const CHAR                  *Format,                \
                        ...                                                     \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Remove,                                                 \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction OPTIONAL,   \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node                    \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Directory,                                              \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction OPTIONAL,   \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node,                   \
                        OUT PCHAR                       *Value                  \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        TransactionStart,                                       \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        OUT PXENBUS_STORE_TRANSACTION   *Transaction            \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        TransactionEnd,                                         \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_TRANSACTION   Transaction,            \
                        IN  BOOLEAN                     Commit                  \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Watch,                                                  \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PCHAR                       Prefix OPTIONAL,        \
                        IN  PCHAR                       Node,                   \
                        IN  PKEVENT                     Event,                  \
                        OUT PXENBUS_STORE_WATCH         *Watch                  \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(NTSTATUS,                                               \
                        Unwatch,                                                \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context,                \
                        IN  PXENBUS_STORE_WATCH         Watch                   \
                        )                                                       \
                        )                                                       \
        STORE_OPERATION(VOID,                                                   \
                        Poll,                                                   \
                        (                                                       \
                        IN  PXENBUS_STORE_CONTEXT       Context                 \
                        )                                                       \
                        )

typedef struct _XENBUS_STORE_CONTEXT    XENBUS_STORE_CONTEXT, *PXENBUS_STORE_CONTEXT;

#define STORE_OPERATION(_Type, _Name, _Arguments) \
        _Type (*STORE_ ## _Name) _Arguments;

typedef struct _XENBUS_STORE_OPERATIONS {
    DEFINE_STORE_OPERATIONS
} XENBUS_STORE_OPERATIONS, *PXENBUS_STORE_OPERATIONS;

#undef STORE_OPERATION

typedef struct _XENBUS_STORE_INTERFACE   XENBUS_STORE_INTERFACE, *PXENBUS_STORE_INTERFACE;

// {916920F1-F9EE-465d-8137-5CC61786B840}
DEFINE_GUID(GUID_STORE_INTERFACE,
            0x916920f1,
            0xf9ee,
            0x465d,
            0x81,
            0x37,
            0x5c,
            0xc6,
            0x17,
            0x86,
            0xb8,
            0x40);

#define STORE_INTERFACE_VERSION 4

#define STORE_OPERATIONS(_Interface) \
        (PXENBUS_STORE_OPERATIONS *)((ULONG_PTR)(_Interface))

#define STORE_CONTEXT(_Interface) \
        (PXENBUS_STORE_CONTEXT *)((ULONG_PTR)(_Interface) + sizeof (PVOID))

#define STORE(_Operation, _Interface, ...) \
        (*STORE_OPERATIONS(_Interface))->STORE_ ## _Operation((*STORE_CONTEXT(_Interface)), __VA_ARGS__)

#endif  // _XENBUS_STORE_INTERFACE_H

