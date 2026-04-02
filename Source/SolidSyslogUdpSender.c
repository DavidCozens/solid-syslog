#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDef.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
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

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    struct SolidSyslogUdpSender* self = malloc(sizeof(struct SolidSyslogUdpSender));
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->config = *config;
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->base.Send = Send;
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->fd = socket(AF_INET, SOCK_DGRAM, 0);
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->addr = ResolveAddress(config);

    return &self->base;
}

void SolidSyslogUdpSender_Destroy(struct SolidSyslogSender* sender)
{
    struct SolidSyslogUdpSender* self = (struct SolidSyslogUdpSender*) sender;
    close(self->fd);
    free(self);
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
    struct SolidSyslogUdpSender* sender = (struct SolidSyslogUdpSender*) self;
    sendto(sender->fd, buffer, size, 0, (struct sockaddr*) &sender->addr, sizeof(sender->addr));
}
