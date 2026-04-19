#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogAddressInternal.h"
#include "SolidSyslogResolverDefinition.h"

#include <netdb.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>

enum
{
    GETADDRINFO_SUCCESS = 0
};

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result);
static bool ResolveAt(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, const char* host, uint16_t port,
                      struct SolidSyslogAddress* result);

static int MapTransport(enum SolidSyslogTransport transport);

struct SolidSyslogGetAddrInfoResolver
{
    struct SolidSyslogResolver base;
    const char* (*getHost)(void);
    int (*getPort)(void);
};

static struct SolidSyslogGetAddrInfoResolver instance;

struct SolidSyslogResolver* SolidSyslogGetAddrInfoResolver_Create(const char* (*getHost)(void), int (*getPort)(void))
{
    instance.base.Resolve   = Resolve;
    instance.base.ResolveAt = ResolveAt;
    instance.getHost        = getHost;
    instance.getPort        = getPort;
    return &instance.base;
}

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result)
{
    struct SolidSyslogGetAddrInfoResolver* resolver = (struct SolidSyslogGetAddrInfoResolver*) self;
    return ResolveAt(self, transport, resolver->getHost(), (uint16_t) resolver->getPort(), result);
}

static bool ResolveAt(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, const char* host, uint16_t port, struct SolidSyslogAddress* result)
{
    (void) self;

    struct addrinfo hints = {0};
    hints.ai_family       = AF_INET;
    hints.ai_socktype     = MapTransport(transport);

    struct addrinfo* info     = NULL;
    bool             resolved = false;

    if (getaddrinfo(host, NULL, &hints, &info) == GETADDRINFO_SUCCESS)
    {
        struct sockaddr_in* sin = SolidSyslogAddress_AsSockaddrIn(result);
        *sin                    = *(struct sockaddr_in*) info->ai_addr;
        sin->sin_port           = htons(port);
        freeaddrinfo(info);
        resolved = true;
    }

    return resolved;
}

static int MapTransport(enum SolidSyslogTransport transport)
{
    int socktype = SOCK_DGRAM;

    if (transport == SOLIDSYSLOG_TRANSPORT_TCP)
    {
        socktype = SOCK_STREAM;
    }

    return socktype;
}

void SolidSyslogGetAddrInfoResolver_Destroy(void)
{
    instance.base.Resolve   = NULL;
    instance.base.ResolveAt = NULL;
    instance.getHost        = NULL;
    instance.getPort        = NULL;
}
