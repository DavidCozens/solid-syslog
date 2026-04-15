#include "SolidSyslogUdpSender.h"
#include "SolidSyslogResolverDefinition.h"
#include "SolidSyslogSenderDefinition.h"

#include <sys/socket.h>
#include <unistd.h>

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender          base;
    int                               fd;
    struct SolidSyslogUdpSenderConfig config;
    struct sockaddr_in                addr;
};

static struct SolidSyslogUdpSender instance = {.fd = -1};

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.fd        = socket(AF_INET, SOCK_DGRAM, 0);

    config->resolver->Resolve(config->resolver, config->getHost(), SOCK_DGRAM, &instance.addr);
    instance.addr.sin_port = htons((uint16_t) config->getPort());

    return &instance.base;
}

void SolidSyslogUdpSender_Destroy(void)
{
    if (instance.fd >= 0)
    {
        close(instance.fd);
    }
    instance.fd        = -1;
    instance.base.Send = NULL;
    instance.addr      = (struct sockaddr_in) {0};
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* udpSender = (struct SolidSyslogUdpSender*) self;
    return sendto(udpSender->fd, buffer, size, 0, (struct sockaddr*) &udpSender->addr, sizeof(udpSender->addr)) >= 0;
}
