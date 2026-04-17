#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDefinition.h"

#include <stddef.h>

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size);

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender          base;
    struct SolidSyslogUdpSenderConfig config;
    SolidSyslogAddressStorage         addrStorage;
    bool                              ready;
};

static struct SolidSyslogUdpSender instance;

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;

    struct SolidSyslogAddress* addr = SolidSyslogAddress_FromStorage(&instance.addrStorage);

    bool opened   = SolidSyslogDatagram_Open(config->datagram);
    bool resolved = false;

    if (opened)
    {
        resolved = SolidSyslogResolver_Resolve(config->resolver, SOLIDSYSLOG_TRANSPORT_UDP, addr);
    }

    instance.ready = opened && resolved;

    return &instance.base;
}

void SolidSyslogUdpSender_Destroy(void)
{
    if (instance.config.datagram != NULL)
    {
        SolidSyslogDatagram_Close(instance.config.datagram);
    }
    instance.base.Send = NULL;
    instance.addrStorage = (SolidSyslogAddressStorage) {0};
    instance.config    = (struct SolidSyslogUdpSenderConfig) {0};
    instance.ready     = false;
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* udpSender = (struct SolidSyslogUdpSender*) self;
    bool                         sent      = false;

    if (udpSender->ready)
    {
        struct SolidSyslogAddress* addr = SolidSyslogAddress_FromStorage(&udpSender->addrStorage);
        sent = SolidSyslogDatagram_SendTo(udpSender->config.datagram, buffer, size, addr);
    }

    return sent;
}
