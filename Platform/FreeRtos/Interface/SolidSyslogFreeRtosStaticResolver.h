#ifndef SOLIDSYSLOGFREERTOSSTATICRESOLVER_H
#define SOLIDSYSLOGFREERTOSSTATICRESOLVER_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver;

    struct SolidSyslogResolver* SolidSyslogFreeRtosStaticResolver_Create(const uint8_t ipv4Octets[4]);
    void SolidSyslogFreeRtosStaticResolver_Destroy(struct SolidSyslogResolver * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGFREERTOSSTATICRESOLVER_H */
