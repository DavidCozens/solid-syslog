#include "SolidSyslogResolverDefinition.h"

bool SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver, enum SolidSyslogTransport transport, const char* host, uint16_t port,
                                 struct SolidSyslogAddress* result)
{
    return resolver->Resolve(resolver, transport, host, port, result);
}
