#ifndef EXAMPLEUDPCONFIG_H
#define EXAMPLEUDPCONFIG_H

#include "ExternC.h"

EXTERN_C_BEGIN

    const char* ExampleUdpConfig_GetHost(void);
    int         ExampleUdpConfig_GetPort(void);

EXTERN_C_END

#endif /* EXAMPLEUDPCONFIG_H */
