#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDef.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

static struct sockaddr_in BuildAddress(const struct addrinfo* resolved, int port);
static struct sockaddr_in ResolveAddress(const struct SolidSyslogUdpSenderConfig* config);
static void               Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender          base;
    int                               fd;
    struct SolidSyslogUdpSenderConfig config;
    struct sockaddr_in                addr;
};

static struct SolidSyslogUdpSender instance;

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.fd        = socket(AF_INET, SOCK_DGRAM, 0);
    instance.addr      = ResolveAddress(config);

    return &instance.base;
}

void SolidSyslogUdpSender_Destroy(struct SolidSyslogSender* sender)
{
    (void) sender;
    close(instance.fd);
    instance.fd        = -1;
    instance.base.Send = NULL;
    instance.addr      = (struct sockaddr_in) {0};
}

static struct sockaddr_in ResolveAddress(const struct SolidSyslogUdpSenderConfig* config)
{
    struct addrinfo  hints  = {0};
    struct addrinfo* result = NULL;
    hints.ai_family         = AF_INET;
    hints.ai_socktype       = SOCK_DGRAM;

    // NOLINTNEXTLINE(bugprone-unused-return-value) -- error handling deferred to error handling phase
    getaddrinfo(config->getHost(), NULL, &hints, &result);

    struct sockaddr_in addr = BuildAddress(result, config->getPort());
    freeaddrinfo(result);

    return addr;
}

static struct sockaddr_in BuildAddress(const struct addrinfo* resolved, int port)
{
    struct sockaddr_in addr = *(struct sockaddr_in*) resolved->ai_addr;
    addr.sin_port           = htons((uint16_t) port);
    return addr;
}

static void Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* udpSender = (struct SolidSyslogUdpSender*) self;
    sendto(udpSender->fd, buffer, size, 0, (struct sockaddr*) &udpSender->addr, sizeof(udpSender->addr));
}
