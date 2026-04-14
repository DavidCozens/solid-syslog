#include "SolidSyslogTcpSender.h"
#include "SolidSyslogFormatter.h"
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

enum
{
    UINT32_MAX_DECIMAL_DIGITS      = 10,
    OCTET_COUNTING_SEPARATOR       = 1,
    OCTET_COUNTING_NULL_TERMINATOR = 1,
    OCTET_COUNTING_PREFIX_CAPACITY = UINT32_MAX_DECIMAL_DIGITS + OCTET_COUNTING_SEPARATOR + OCTET_COUNTING_NULL_TERMINATOR
};

static bool                         Connect(struct SolidSyslogTcpSender* tcp);
static void                         CreateSocket(struct SolidSyslogTcpSender* tcp);
static void                         Disconnect(struct SolidSyslogTcpSender* tcp);
static bool                         EnsureConnected(struct SolidSyslogTcpSender* tcp);
static struct SolidSyslogFormatter* FormatOctetCountingPrefix(SolidSyslogFormatterStorage* storage, size_t messageSize);
static bool                         Send(struct SolidSyslogSender* self, const void* buffer, size_t size);
static bool                         SendData(struct SolidSyslogTcpSender* tcp, const void* data, size_t len);
static struct sockaddr_in           BuildAddress(const struct addrinfo* resolved, int port);
static struct sockaddr_in           ResolveAddress(const struct SolidSyslogTcpSenderConfig* config);
static void                         EnableTcpNoDelay(int fd);

struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config)
{
    instance.config    = *config;
    instance.base.Send = Send;
    instance.connected = false;
    instance.fd        = -1;
    return &instance.base;
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
    CreateSocket(tcp);

    struct sockaddr_in addr = ResolveAddress(&tcp->config);
    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions) -- socket() failure handling deferred to error handling epic
    bool connected = connect(tcp->fd, (struct sockaddr*) &addr, sizeof(addr)) == 0;

    if (connected)
    {
        tcp->connected = true;
    }
    else
    {
        Disconnect(tcp);
    }

    return connected;
}

static void CreateSocket(struct SolidSyslogTcpSender* tcp)
{
    tcp->fd = socket(AF_INET, SOCK_STREAM, 0);
    EnableTcpNoDelay(tcp->fd);
}

static void EnableTcpNoDelay(int fd)
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

static struct sockaddr_in ResolveAddress(const struct SolidSyslogTcpSenderConfig* config)
{
    struct addrinfo  hints  = {0};
    struct addrinfo* result = NULL;
    hints.ai_family         = AF_INET;
    hints.ai_socktype       = SOCK_STREAM;

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

static void Disconnect(struct SolidSyslogTcpSender* tcp)
{
    close(tcp->fd);
    tcp->fd        = -1;
    tcp->connected = false;
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
    bool sent = send(tcp->fd, data, len, MSG_NOSIGNAL) >= 0;

    if (!sent)
    {
        Disconnect(tcp);
    }

    return sent;
}
