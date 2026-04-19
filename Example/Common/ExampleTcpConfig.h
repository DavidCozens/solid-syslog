#ifndef EXAMPLETCPCONFIG_H
#define EXAMPLETCPCONFIG_H

#include "ExternC.h"
#include "SolidSyslogEndpoint.h"

EXTERN_C_BEGIN

    const char* ExampleTcpConfig_GetHost(void);
    int         ExampleTcpConfig_GetPort(void);
    void        ExampleTcpConfig_GetEndpoint(struct SolidSyslogEndpoint * endpoint);

EXTERN_C_END

#endif /* EXAMPLETCPCONFIG_H */
