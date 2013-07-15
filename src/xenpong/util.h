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
