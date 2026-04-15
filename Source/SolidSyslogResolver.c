#include "SolidSyslogResolverDefinition.h"

void SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver, enum SolidSyslogTransport transport, struct sockaddr_in* result)
{
    resolver->Resolve(resolver, transport, result);
}
