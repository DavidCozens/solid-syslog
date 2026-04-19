#ifndef SOLIDSYSLOGGETADDRINFORESOLVERH
#define SOLIDSYSLOGGETADDRINFORESOLVERH

#include "SolidSyslogResolver.h"

EXTERN_C_BEGIN

    struct SolidSyslogResolver* SolidSyslogGetAddrInfoResolver_Create(void);
    void                        SolidSyslogGetAddrInfoResolver_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGGETADDRINFORESOLVERH */
