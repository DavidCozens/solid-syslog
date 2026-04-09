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

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

struct SolidSyslogTcpSender
{
    struct SolidSyslogSender          base;
    int                               fd;
    struct SolidSyslogTcpSenderConfig config;
    bool                              connected;
};

static struct SolidSyslogTcpSender instance = {.fd = -1};

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

struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.connected = false;
    instance.fd        = socket(AF_INET, SOCK_STREAM, 0);
    int enable         = 1;
    setsockopt(instance.fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    return &instance.base;
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogTcpSender* tcp = (struct SolidSyslogTcpSender*) self;

    if (!tcp->connected)
    {
        struct sockaddr_in addr = ResolveAddress(&tcp->config);
        // NOLINTNEXTLINE(bugprone-unused-return-value) -- error handling deferred to S15.2
        connect(tcp->fd, (struct sockaddr*) &addr, sizeof(addr));
        tcp->connected = true;
    }

    char   prefix[12];
    size_t prefixLen = SolidSyslogFormat_Uint32(prefix, (uint32_t) size);
    prefixLen += SolidSyslogFormat_Character(prefix + prefixLen, ' ');
    send(tcp->fd, prefix, prefixLen, 0);
    send(tcp->fd, buffer, size, 0);

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
