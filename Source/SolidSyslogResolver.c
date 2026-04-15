#include "SolidSyslogResolverDefinition.h"

void SolidSyslogResolver_Resolve(struct SolidSyslogResolver* resolver,
                                  const char* host,
                                  int socktype,
                                  struct sockaddr_in* result)
{
    resolver->Resolve(resolver, host, socktype, result);
}
