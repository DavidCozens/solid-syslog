#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogResolverDefinition.h"

#include <netdb.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>

enum
{
    GETADDRINFO_SUCCESS = 0
};

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct sockaddr_in* result);

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
    instance.base.Resolve = Resolve;
    instance.getHost      = getHost;
    instance.getPort      = getPort;
    return &instance.base;
}

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct sockaddr_in* result)
{
    struct SolidSyslogGetAddrInfoResolver* resolver = (struct SolidSyslogGetAddrInfoResolver*) self;

    struct addrinfo hints = {0};
    hints.ai_family       = AF_INET;
    hints.ai_socktype     = MapTransport(transport);

    struct addrinfo* info     = NULL;
    bool             resolved = false;

    if (getaddrinfo(resolver->getHost(), NULL, &hints, &info) == GETADDRINFO_SUCCESS)
    {
        *result          = *(struct sockaddr_in*) info->ai_addr;
        result->sin_port = htons((uint16_t) resolver->getPort());
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
    instance.base.Resolve = NULL;
    instance.getHost      = NULL;
    instance.getPort      = NULL;
}
