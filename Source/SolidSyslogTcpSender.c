#include "SolidSyslogTcpSender.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogSenderDefinition.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>

struct SolidSyslogTcpSender
{
    struct SolidSyslogSender          base;
    int                               fd;
    struct SolidSyslogTcpSenderConfig config;
    bool                              connected;
};

static struct SolidSyslogTcpSender instance = {.fd = -1};

static bool Connect(struct SolidSyslogTcpSender* tcp);
static void CreateSocket(struct SolidSyslogTcpSender* tcp);
static void Disconnect(struct SolidSyslogTcpSender* tcp);
static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

static struct sockaddr_in ResolveAddress(const struct SolidSyslogTcpSenderConfig* config)
{
    struct addrinfo  hints  = {0};
    struct addrinfo* result = NULL;
    hints.ai_family         = AF_INET;
    hints.ai_socktype       = SOCK_STREAM;

    // NOLINTNEXTLINE(bugprone-unused-return-value) -- error handling deferred to error handling phase
    getaddrinfo(config->getHost(), NULL, &hints, &result);

    struct sockaddr_in addr = *(struct sockaddr_in*) result->ai_addr;
    addr.sin_port           = htons((uint16_t) config->getPort());
    freeaddrinfo(result);

    return addr;
}

static void CreateSocket(struct SolidSyslogTcpSender* tcp)
{
    tcp->fd    = socket(AF_INET, SOCK_STREAM, 0);
    int enable = 1;
    setsockopt(tcp->fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

static void Disconnect(struct SolidSyslogTcpSender* tcp)
{
    close(tcp->fd);
    tcp->fd        = -1;
    tcp->connected = false;
}

static bool Connect(struct SolidSyslogTcpSender* tcp)
{
    CreateSocket(tcp);

    struct sockaddr_in addr = ResolveAddress(&tcp->config);
    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions) -- socket() failure handling deferred to error handling epic
    if (connect(tcp->fd, (struct sockaddr*) &addr, sizeof(addr)) != 0)
    {
        return false;
    }

    tcp->connected = true;
    return true;
}

struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.connected = false;
    instance.fd        = -1;
    return &instance.base;
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogTcpSender* tcp = (struct SolidSyslogTcpSender*) self;

    if (!tcp->connected)
    {
        if (!Connect(tcp))
        {
            return false;
        }
    }

    char   prefix[12];
    size_t prefixLen = SolidSyslogFormat_Uint32(prefix, (uint32_t) size);
    prefixLen += SolidSyslogFormat_Character(prefix + prefixLen, ' ');

    if (send(tcp->fd, prefix, prefixLen, MSG_NOSIGNAL) < 0)
    {
        Disconnect(tcp);
        return false;
    }

    if (send(tcp->fd, buffer, size, MSG_NOSIGNAL) < 0)
    {
        Disconnect(tcp);
        return false;
    }

    return true;
}

void SolidSyslogTcpSender_Destroy(void)
{
    if (instance.fd >= 0)
    {
        close(instance.fd);
    }
    instance.fd        = -1;
    instance.base.Send = NULL;
    instance.connected = false;
}
