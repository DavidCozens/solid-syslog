#ifndef EXAMPLETLSCONFIG_H
#define EXAMPLETLSCONFIG_H

#include "ExternC.h"
#include "SolidSyslogEndpoint.h"

#include <stdint.h>

EXTERN_C_BEGIN

    const char* ExampleTlsConfig_GetHost(void);
    int         ExampleTlsConfig_GetPort(void);
    const char* ExampleTlsConfig_GetCaBundlePath(void);
    const char* ExampleTlsConfig_GetServerName(void);
    void        ExampleTlsConfig_GetEndpoint(struct SolidSyslogEndpoint * endpoint);
    uint32_t    ExampleTlsConfig_GetEndpointVersion(void);

EXTERN_C_END

#endif /* EXAMPLETLSCONFIG_H */
