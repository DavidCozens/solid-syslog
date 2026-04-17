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

static struct SolidSyslogTcpSender instance;

enum
{
    UINT32_MAX_DECIMAL_DIGITS      = 10,
    OCTET_COUNTING_SEPARATOR       = 1,
    OCTET_COUNTING_NULL_TERMINATOR = 1,
    OCTET_COUNTING_PREFIX_CAPACITY = UINT32_MAX_DECIMAL_DIGITS + OCTET_COUNTING_SEPARATOR + OCTET_COUNTING_NULL_TERMINATOR
};

static bool                         Connect(struct SolidSyslogTcpSender* tcp);
static bool                         EnsureConnected(struct SolidSyslogTcpSender* tcp);
static struct SolidSyslogFormatter* FormatOctetCountingPrefix(SolidSyslogFormatterStorage* storage, size_t messageSize);
static bool                         Send(struct SolidSyslogSender* self, const void* buffer, size_t size);
static bool                         SendData(struct SolidSyslogTcpSender* tcp, const void* data, size_t len);

struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.connected = false;
    return &instance.base;
}

void SolidSyslogTcpSender_Destroy(void)
{
    if (instance.connected && instance.config.stream != NULL)
    {
        SolidSyslogStream_Close(instance.config.stream);
    }
    instance.base.Send = NULL;
    instance.connected = false;
    instance.config    = (struct SolidSyslogTcpSenderConfig) {0};
}

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogTcpSender* tcp  = (struct SolidSyslogTcpSender*) self;
    bool                         sent = false;

    if (EnsureConnected(tcp))
    {
        SolidSyslogFormatterStorage  prefixStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(OCTET_COUNTING_PREFIX_CAPACITY)];
        struct SolidSyslogFormatter* prefix = FormatOctetCountingPrefix(prefixStorage, size);

        sent = SendData(tcp, SolidSyslogFormatter_AsString(prefix), SolidSyslogFormatter_Length(prefix)) && SendData(tcp, buffer, size);
    }

    return sent;
}

static bool EnsureConnected(struct SolidSyslogTcpSender* tcp)
{
    bool connected = tcp->connected;

    if (!connected)
    {
        connected = Connect(tcp);
    }

    return connected;
}

static bool Connect(struct SolidSyslogTcpSender* tcp)
{
    struct sockaddr_in addr;
    SolidSyslogResolver_Resolve(tcp->config.resolver, SOLIDSYSLOG_TRANSPORT_TCP, &addr);

    tcp->connected = SolidSyslogStream_Open(tcp->config.stream, &addr);

    return tcp->connected;
}

static struct SolidSyslogFormatter* FormatOctetCountingPrefix(SolidSyslogFormatterStorage* storage, size_t messageSize)
{
    struct SolidSyslogFormatter* f = SolidSyslogFormatter_Create(storage, OCTET_COUNTING_PREFIX_CAPACITY);
    SolidSyslogFormatter_Uint32(f, (uint32_t) messageSize);
    SolidSyslogFormatter_Character(f, ' ');
    return f;
}

static bool SendData(struct SolidSyslogTcpSender* tcp, const void* data, size_t len)
{
    bool sent = SolidSyslogStream_Send(tcp->config.stream, data, len);

    if (!sent)
    {
        SolidSyslogStream_Close(tcp->config.stream);
        tcp->connected = false;
    }

    return sent;
}
