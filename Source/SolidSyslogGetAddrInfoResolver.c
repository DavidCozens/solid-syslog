#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"

#include <netdb.h>
#include <stddef.h>
#include <sys/socket.h>

static void Resolve(struct SolidSyslogResolver* self,
                    enum SolidSyslogTransport transport,
                    struct sockaddr_in* result);

static int MapTransport(enum SolidSyslogTransport transport);

struct SolidSyslogGetAddrInfoResolver
{
    struct SolidSyslogResolver base;
    const char* (*getHost)(void);
    int (*getPort)(void);
};

static struct SolidSyslogGetAddrInfoResolver instance;

struct SolidSyslogResolver* SolidSyslogGetAddrInfoResolver_Create(
    const char* (*getHost)(void),
    int (*getPort)(void))
{
    instance.base.Resolve = Resolve;
    instance.getHost      = getHost;
    instance.getPort      = getPort;
    return &instance.base;
}

static void Resolve(struct SolidSyslogResolver* self,
                    enum SolidSyslogTransport transport,
                    struct sockaddr_in* result)
{
    struct SolidSyslogGetAddrInfoResolver* resolver = (struct SolidSyslogGetAddrInfoResolver*) self;

    struct addrinfo  hints  = {0};
    struct addrinfo* resolved = NULL;
    hints.ai_family           = AF_INET;
    hints.ai_socktype         = MapTransport(transport);

    // NOLINTNEXTLINE(bugprone-unused-return-value) -- error handling deferred to error handling phase
    getaddrinfo(resolver->getHost(), NULL, &hints, &resolved);

    *result = *(struct sockaddr_in*) resolved->ai_addr;
    result->sin_port = htons((uint16_t) resolver->getPort());
    freeaddrinfo(resolved);
}

static int MapTransport(enum SolidSyslogTransport transport)
{
    (void) transport;
    return SOCK_DGRAM;
}

void SolidSyslogGetAddrInfoResolver_Destroy(void)
{
    instance.base.Resolve = NULL;
    instance.getHost      = NULL;
    instance.getPort      = NULL;
}
