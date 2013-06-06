#if !defined(_LOG_H_)
#define _LOG_H_

VOID
__Warning(
    IN  const CHAR  *Prefix,
    IN  const CHAR  *Format,
    ...
    );

#define Warning(...)  \
        __Warning(__MODULE__ "|" __FUNCTION__ ": ", __VA_ARGS__)

#endif  // _LOG_H_