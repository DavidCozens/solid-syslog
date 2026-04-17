#include "SolidSyslogResolverDefinition.h"

bool SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver, enum SolidSyslogTransport transport, struct sockaddr_in* result)
{
    return resolver->Resolve(resolver, transport, result);
}
