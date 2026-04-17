#include "SolidSyslogWinsockResolver.h"
#include "SolidSyslogAddressInternal.h"
#include "SolidSyslogResolverDefinition.h"
#include "SolidSyslogWinsockResolverInternal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* File-local forwarders. Taking the address of a __declspec(dllimport)
   Winsock function for static initialisation triggers MSVC C4232 (the address
   isn't a compile-time constant); forwarding through a static function whose
   address IS a compile-time constant avoids the warning without a suppression. */
static int WSAAPI  CallGetAddrInfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res);
static void WSAAPI CallFreeAddrInfo(struct addrinfo* res);

WinsockGetAddrInfoFn  Winsock_getaddrinfo  = CallGetAddrInfo;
WinsockFreeAddrInfoFn Winsock_freeaddrinfo = CallFreeAddrInfo;

static int WSAAPI CallGetAddrInfo(const char* node, const char* service, const struct addrinfo* hints, struct addrinfo** res)
{
    return getaddrinfo(node, service, hints, res);
}

static void WSAAPI CallFreeAddrInfo(struct addrinfo* res)
{
    freeaddrinfo(res);
}

enum
{
    GETADDRINFO_SUCCESS = 0
};

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result);

static int MapTransport(enum SolidSyslogTransport transport);

struct SolidSyslogWinsockResolver
{
    struct SolidSyslogResolver base;
    const char* (*getHost)(void);
    int (*getPort)(void);
};

static struct SolidSyslogWinsockResolver instance;

struct SolidSyslogResolver* SolidSyslogWinsockResolver_Create(const char* (*getHost)(void), int (*getPort)(void))
{
    instance.base.Resolve = Resolve;
    instance.getHost      = getHost;
    instance.getPort      = getPort;
    return &instance.base;
}

static bool Resolve(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result)
{
    struct SolidSyslogWinsockResolver* resolver = (struct SolidSyslogWinsockResolver*) self;

    struct addrinfo hints = {0};
    hints.ai_family       = AF_INET;
    hints.ai_socktype     = MapTransport(transport);

    struct addrinfo* info     = NULL;
    bool             resolved = false;

    if (Winsock_getaddrinfo(resolver->getHost(), NULL, &hints, &info) == GETADDRINFO_SUCCESS)
    {
        struct sockaddr_in* sin = SolidSyslogAddress_AsSockaddrIn(result);
        *sin                    = *(struct sockaddr_in*) info->ai_addr;
        sin->sin_port           = htons((uint16_t) resolver->getPort());
        Winsock_freeaddrinfo(info);
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

void SolidSyslogWinsockResolver_Destroy(void)
{
    instance.base.Resolve = NULL;
    instance.getHost      = NULL;
    instance.getPort      = NULL;
}
