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

static bool               Connect(struct SolidSyslogTcpSender* tcp);
static void               CreateSocket(struct SolidSyslogTcpSender* tcp);
static void               Disconnect(struct SolidSyslogTcpSender* tcp);
static bool               EnsureConnected(struct SolidSyslogTcpSender* tcp);
static size_t             FormatOctetCountingPrefix(char* prefix, size_t messageSize);
static bool               Send(struct SolidSyslogSender* self, const void* buffer, size_t size);
static bool               SendData(struct SolidSyslogTcpSender* tcp, const void* data, size_t len);
static struct sockaddr_in BuildAddress(const struct addrinfo* resolved, int port);
static struct sockaddr_in ResolveAddress(const struct SolidSyslogTcpSenderConfig* config);
static void               EnableTcpNoDelay(int fd);

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
    struct SolidSyslogTcpSender* tcp    = (struct SolidSyslogTcpSender*) self;
    bool                         result = EnsureConnected(tcp);

    if (result)
    {
        char   prefix[OCTET_COUNTING_PREFIX_CAPACITY];
        size_t prefixLen = FormatOctetCountingPrefix(prefix, size);

        result = SendData(tcp, prefix, prefixLen) && SendData(tcp, buffer, size);
    }

    return result;
}

static bool EnsureConnected(struct SolidSyslogTcpSender* tcp)
{
    bool ready = tcp->connected;

    if (!ready)
    {
        ready = Connect(tcp);
    }

    return ready;
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

static size_t FormatOctetCountingPrefix(char* prefix, size_t messageSize)
{
    size_t len = SolidSyslogFormat_Uint32(prefix, (uint32_t) messageSize);
    len += SolidSyslogFormat_Character(prefix + len, ' ');
    return len;
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
