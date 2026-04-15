#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"

#include <netdb.h>
#include <stddef.h>

static void Resolve(struct SolidSyslogResolver* self,
                    const char* host,
                    int socktype,
                    struct sockaddr_in* result);

struct SolidSyslogGetAddrInfoResolver
{
    struct SolidSyslogResolver base;
};

static struct SolidSyslogGetAddrInfoResolver instance;

struct SolidSyslogResolver* SolidSyslogGetAddrInfoResolver_Create(void)
{
    instance.base.Resolve = Resolve;
    return &instance.base;
}

static void Resolve(struct SolidSyslogResolver* self,
                    const char* host,
                    int socktype,
                    struct sockaddr_in* result)
{
    (void) self;

    struct addrinfo  hints  = {0};
    struct addrinfo* resolved = NULL;
    hints.ai_family           = AF_INET;
    hints.ai_socktype         = socktype;

    // NOLINTNEXTLINE(bugprone-unused-return-value) -- error handling deferred to error handling phase
    getaddrinfo(host, NULL, &hints, &resolved);

    *result = *(struct sockaddr_in*) resolved->ai_addr;
    result->sin_port = 0;
    freeaddrinfo(resolved);
}

void SolidSyslogGetAddrInfoResolver_Destroy(void)
{
    instance.base.Resolve = NULL;
}
