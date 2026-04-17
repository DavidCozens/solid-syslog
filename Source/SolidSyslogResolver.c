#include "SolidSyslogResolverDefinition.h"

bool SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result)
{
    return resolver->Resolve(resolver, transport, result);
}
