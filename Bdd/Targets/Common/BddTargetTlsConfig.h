#ifndef BDDTARGETTLSCONFIG_H
#define BDDTARGETTLSCONFIG_H

#include <stdint.h>

#include "ExternC.h"

struct SolidSyslogEndpoint;

EXTERN_C_BEGIN

    const char* BddTargetTlsConfig_GetHost(void);
    uint16_t BddTargetTlsConfig_GetPort(void);
    const char* BddTargetTlsConfig_GetCaBundlePath(void);
    const char* BddTargetTlsConfig_GetServerName(void);
    void BddTargetTlsConfig_GetEndpoint(struct SolidSyslogEndpoint * endpoint);
    uint32_t BddTargetTlsConfig_GetEndpointVersion(void);

    /* Override the default TLS host ("syslog-ng" — Linux compose service
       name). Caller owns the string lifetime. Used by the per-platform
       main.c to inject SOLIDSYSLOG_BDD_TLS_HOST when set, so the same
       example targets the Linux compose oracle or the Windows OTel oracle
       on 127.0.0.1. */
    void BddTargetTlsConfig_SetHost(const char* host);

EXTERN_C_END

#endif /* BDDTARGETTLSCONFIG_H */
