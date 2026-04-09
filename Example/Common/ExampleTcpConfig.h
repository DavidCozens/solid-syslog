#ifndef EXAMPLETCPCONFIG_H
#define EXAMPLETCPCONFIG_H

#include "ExternC.h"

EXTERN_C_BEGIN

    const char* ExampleTcpConfig_GetHost(void);
    int         ExampleTcpConfig_GetPort(void);

EXTERN_C_END

#endif /* EXAMPLETCPCONFIG_H */
