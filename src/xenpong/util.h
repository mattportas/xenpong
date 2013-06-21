#if !defined(_UTIL_H_)
#define _UTIL_H_

#include <ntddk.h>

typedef struct _NON_PAGED_BUFFER_HEADER {
    SIZE_T  Length;
    ULONG   Tag;
} NON_PAGED_BUFFER_HEADER, *PNON_PAGED_BUFFER_HEADER;

typedef struct _NON_PAGED_BUFFER_TRAILER {
    ULONG   Tag;
} NON_PAGED_BUFFER_TRAILER, *PNON_PAGED_BUFFER_TRAILER;

static FORCEINLINE PVOID
__AllocateNonPagedPoolWithTag(
    __in SIZE_T Length,
    __in ULONG Tag
    )
{
    PUCHAR Buffer;
    PNON_PAGED_BUFFER_HEADER Header;
    PNON_PAGED_BUFFER_TRAILER Trailer;

    Buffer = ExAllocatePoolWithTag(NonPagedPool,
                                   sizeof (NON_PAGED_BUFFER_HEADER) +
                                   Length +
                                   sizeof (NON_PAGED_BUFFER_TRAILER),
                                   Tag);
    if (Buffer == NULL) {
        return Buffer;
    }

    RtlZeroMemory(Buffer, 
                  sizeof (NON_PAGED_BUFFER_HEADER) +
                  Length +
                  sizeof (NON_PAGED_BUFFER_TRAILER));

    Header = (PNON_PAGED_BUFFER_HEADER)Buffer;
    Header->Length = Length;
    Header->Tag = Tag;

    Buffer += sizeof (NON_PAGED_BUFFER_HEADER);

    Trailer = (PNON_PAGED_BUFFER_TRAILER)(Buffer + Length);
    Trailer->Tag = Tag;

    return Buffer;
}

static FORCEINLINE VOID
__FreePoolWithTag(
    __in PVOID _Buffer,
    __in ULONG Tag
    )
{
    PUCHAR Buffer = _Buffer;
    SIZE_T Length;
    PNON_PAGED_BUFFER_HEADER Header;
    PNON_PAGED_BUFFER_TRAILER Trailer;

    Buffer -= sizeof (NON_PAGED_BUFFER_HEADER);

    Header = (PNON_PAGED_BUFFER_HEADER)Buffer;
    Length = Header->Length;

    Buffer += sizeof (NON_PAGED_BUFFER_HEADER);

    Trailer = (PNON_PAGED_BUFFER_TRAILER)(Buffer + Length);

    Buffer -= sizeof (NON_PAGED_BUFFER_HEADER);

    RtlFillMemory(Buffer, 
                  sizeof (NON_PAGED_BUFFER_HEADER) +
                  Length +
                  sizeof (NON_PAGED_BUFFER_TRAILER),
                  0xAA);

    ExFreePoolWithTag(Buffer, Tag);
}

#endif  // _UTIL_H_
