#include <ntddk.h>
#include <stdarg.h>

#include "log.h"

VOID
__Warning(
    IN  const CHAR  *Prefix,
    IN  const CHAR  *Format,
    ...
    )
{
    va_list         Arguments;

    va_start(Arguments, Format);

#pragma prefast(suppress:6001) // Using uninitialized memory
    vDbgPrintExWithPrefix(Prefix,
                          DPFLTR_IHVDRIVER_ID,
                          DPFLTR_WARNING_LEVEL,
                          Format,
                          Arguments);
    va_end(Arguments);
}