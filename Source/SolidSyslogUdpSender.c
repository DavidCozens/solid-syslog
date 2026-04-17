#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDefinition.h"

#include <stddef.h>

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender          base;
    struct SolidSyslogUdpSenderConfig config;
    struct sockaddr_in                addr;
};

static struct SolidSyslogUdpSender instance;

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;

    SolidSyslogDatagram_Open(config->datagram);
    SolidSyslogResolver_Resolve(config->resolver, SOLIDSYSLOG_TRANSPORT_UDP, &instance.addr);

    return &instance.base;
}

void SolidSyslogUdpSender_Destroy(void)
{
    if (instance.config.datagram != NULL)
    {
        SolidSyslogDatagram_Close(instance.config.datagram);
    }
    instance.base.Send = NULL;
    instance.addr      = (struct sockaddr_in) {0};
    instance.config    = (struct SolidSyslogUdpSenderConfig) {0};
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* udpSender = (struct SolidSyslogUdpSender*) self;
    return SolidSyslogDatagram_SendTo(udpSender->config.datagram, buffer, size, &udpSender->addr);
}
