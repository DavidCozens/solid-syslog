#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDefinition.h"

#include <stddef.h>

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender          base;
    struct SolidSyslogUdpSenderConfig config;
    SolidSyslogAddressStorage         addrStorage;
    bool                              connected;
};

static bool                              Send(struct SolidSyslogSender* self, const void* buffer, size_t size);
static inline bool                       EnsureConnected(struct SolidSyslogUdpSender* udp);
static inline bool                       Connected(struct SolidSyslogUdpSender* udp);
static bool                              Connect(struct SolidSyslogUdpSender* udp);
static inline bool                       OpenSocket(struct SolidSyslogUdpSender* udp);
static inline bool                       ResolveDestination(struct SolidSyslogUdpSender* udp);
static inline struct SolidSyslogAddress* Address(struct SolidSyslogUdpSender* udp);
static inline void                       CloseSocket(struct SolidSyslogUdpSender* udp);
static inline bool                       TransmitDatagram(struct SolidSyslogUdpSender* udp, const void* buffer, size_t size);

static const struct SolidSyslogUdpSender DEFAULT_INSTANCE = {0};
static struct SolidSyslogUdpSender       instance;

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    instance           = DEFAULT_INSTANCE;
    instance.config    = *config;
    instance.base.Send = Send;
    return &instance.base;
}

void SolidSyslogUdpSender_Destroy(void)
{
    if (Connected(&instance))
    {
        SolidSyslogDatagram_Close(instance.config.datagram);
    }
    instance = DEFAULT_INSTANCE;
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* udp = (struct SolidSyslogUdpSender*) self;
    return EnsureConnected(udp) && TransmitDatagram(udp, buffer, size);
}

static inline bool EnsureConnected(struct SolidSyslogUdpSender* udp)
{
    return Connected(udp) || Connect(udp);
}

static inline bool Connected(struct SolidSyslogUdpSender* udp)
{
    return udp->connected;
}

static bool Connect(struct SolidSyslogUdpSender* udp)
{
    udp->connected = OpenSocket(udp) && ResolveDestination(udp);

    if (!Connected(udp))
    {
        CloseSocket(udp);
    }

    return Connected(udp);
}

static inline bool OpenSocket(struct SolidSyslogUdpSender* udp)
{
    return SolidSyslogDatagram_Open(udp->config.datagram);
}

static inline bool ResolveDestination(struct SolidSyslogUdpSender* udp)
{
    return SolidSyslogResolver_Resolve(udp->config.resolver, SOLIDSYSLOG_TRANSPORT_UDP, Address(udp));
}

static inline struct SolidSyslogAddress* Address(struct SolidSyslogUdpSender* udp)
{
    return SolidSyslogAddress_FromStorage(&udp->addrStorage);
}

static inline void CloseSocket(struct SolidSyslogUdpSender* udp)
{
    SolidSyslogDatagram_Close(udp->config.datagram);
}

static inline bool TransmitDatagram(struct SolidSyslogUdpSender* udp, const void* buffer, size_t size)
{
    return SolidSyslogDatagram_SendTo(udp->config.datagram, buffer, size, Address(udp));
}
