#include "SolidSyslogTcpSender.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogSenderDefinition.h"

#include <stdbool.h>
#include <stddef.h>

struct SolidSyslogTcpSender
{
    struct SolidSyslogSender          base;
    struct SolidSyslogTcpSenderConfig config;
    bool                              connected;
};

enum
{
    UINT32_MAX_DECIMAL_DIGITS      = 10,
    OCTET_COUNTING_SEPARATOR       = 1,
    OCTET_COUNTING_NULL_TERMINATOR = 1,
    OCTET_COUNTING_PREFIX_CAPACITY = UINT32_MAX_DECIMAL_DIGITS + OCTET_COUNTING_SEPARATOR + OCTET_COUNTING_NULL_TERMINATOR
};

static bool                         Send(struct SolidSyslogSender* self, const void* buffer, size_t size);
static inline bool                  EnsureConnected(struct SolidSyslogTcpSender* tcp);
static inline bool                  Connected(struct SolidSyslogTcpSender* tcp);
static bool                         Connect(struct SolidSyslogTcpSender* tcp);
static void                         Disconnect(struct SolidSyslogSender* self);
static inline void                  CloseStream(struct SolidSyslogTcpSender* tcp);
static bool                         TransmitFramed(struct SolidSyslogTcpSender* tcp, const void* buffer, size_t size);
static struct SolidSyslogFormatter* FormatOctetCountingPrefix(SolidSyslogFormatterStorage* storage, size_t messageSize);
static bool                         SendBytes(struct SolidSyslogTcpSender* tcp, const void* data, size_t len);

static const struct SolidSyslogTcpSender DEFAULT_INSTANCE = {0};
static struct SolidSyslogTcpSender       instance;

struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config)
{
    instance                 = DEFAULT_INSTANCE;
    instance.config          = *config;
    instance.base.Send       = Send;
    instance.base.Disconnect = Disconnect;
    return &instance.base;
}

void SolidSyslogTcpSender_Destroy(void)
{
    Disconnect(&instance.base);
    instance = DEFAULT_INSTANCE;
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogTcpSender* tcp = (struct SolidSyslogTcpSender*) self;
    return EnsureConnected(tcp) && TransmitFramed(tcp, buffer, size);
}

static inline bool EnsureConnected(struct SolidSyslogTcpSender* tcp)
{
    return Connected(tcp) || Connect(tcp);
}

static inline bool Connected(struct SolidSyslogTcpSender* tcp)
{
    return tcp->connected;
}

static bool Connect(struct SolidSyslogTcpSender* tcp)
{
    SolidSyslogAddressStorage  addrStorage = {0};
    struct SolidSyslogAddress* addr        = SolidSyslogAddress_FromStorage(&addrStorage);

    if (SolidSyslogResolver_Resolve(tcp->config.resolver, SOLIDSYSLOG_TRANSPORT_TCP, addr))
    {
        tcp->connected = SolidSyslogStream_Open(tcp->config.stream, addr);
    }

    return Connected(tcp);
}

static void Disconnect(struct SolidSyslogSender* self)
{
    struct SolidSyslogTcpSender* tcp = (struct SolidSyslogTcpSender*) self;

    if (Connected(tcp))
    {
        CloseStream(tcp);
    }
}

static inline void CloseStream(struct SolidSyslogTcpSender* tcp)
{
    SolidSyslogStream_Close(tcp->config.stream);
    tcp->connected = false;
}

static bool TransmitFramed(struct SolidSyslogTcpSender* tcp, const void* buffer, size_t size)
{
    SolidSyslogFormatterStorage  prefixStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(OCTET_COUNTING_PREFIX_CAPACITY)];
    struct SolidSyslogFormatter* prefix = FormatOctetCountingPrefix(prefixStorage, size);

    return SendBytes(tcp, SolidSyslogFormatter_AsString(prefix), SolidSyslogFormatter_Length(prefix)) && SendBytes(tcp, buffer, size);
}

static struct SolidSyslogFormatter* FormatOctetCountingPrefix(SolidSyslogFormatterStorage* storage, size_t messageSize)
{
    struct SolidSyslogFormatter* f = SolidSyslogFormatter_Create(storage, OCTET_COUNTING_PREFIX_CAPACITY);
    SolidSyslogFormatter_Uint32(f, (uint32_t) messageSize);
    SolidSyslogFormatter_Character(f, ' ');
    return f;
}

static bool SendBytes(struct SolidSyslogTcpSender* tcp, const void* data, size_t len)
{
    bool sent = SolidSyslogStream_Send(tcp->config.stream, data, len);

    if (!sent)
    {
        CloseStream(tcp);
    }

    return sent;
}
