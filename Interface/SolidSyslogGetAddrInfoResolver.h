#ifndef SOLIDSYSLOGGETADDRINFORESOLVERH
#define SOLIDSYSLOGGETADDRINFORESOLVERH

#include "SolidSyslogResolver.h"

EXTERN_C_BEGIN

    struct SolidSyslogResolver* SolidSyslogGetAddrInfoResolver_Create(
        const char* (*getHost)(void),  // NOLINT(modernize-redundant-void-arg) -- C idiom
        int (*getPort)(void));         // NOLINT(modernize-redundant-void-arg) -- C idiom
    void SolidSyslogGetAddrInfoResolver_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGGETADDRINFORESOLVERH */
