#ifndef SOLIDSYSLOGFREERTOSSTATICRESOLVERPRIVATE_H
#define SOLIDSYSLOGFREERTOSSTATICRESOLVERPRIVATE_H

#include <stdint.h>

#include "SolidSyslogResolverDefinition.h"

struct SolidSyslogFreeRtosStaticResolver
{
    struct SolidSyslogResolver Base;
    uint8_t Octets[4];
};

void FreeRtosStaticResolver_Initialise(struct SolidSyslogResolver* base, const uint8_t ipv4Octets[4]);
void FreeRtosStaticResolver_Cleanup(struct SolidSyslogResolver* base);

#endif /* SOLIDSYSLOGFREERTOSSTATICRESOLVERPRIVATE_H */
