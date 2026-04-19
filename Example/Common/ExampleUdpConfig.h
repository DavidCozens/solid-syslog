#ifndef EXAMPLEUDPCONFIG_H
#define EXAMPLEUDPCONFIG_H

#include "ExternC.h"
#include "SolidSyslogEndpoint.h"

EXTERN_C_BEGIN

    const char* ExampleUdpConfig_GetHost(void);
    int         ExampleUdpConfig_GetPort(void);
    void        ExampleUdpConfig_GetEndpoint(struct SolidSyslogEndpoint * endpoint);

EXTERN_C_END

#endif /* EXAMPLEUDPCONFIG_H */
