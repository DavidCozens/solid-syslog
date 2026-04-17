#include "SolidSyslogPosixDatagram.h"
#include "SolidSyslogDatagramDefinition.h"

#include <stdbool.h>
#include <stddef.h>
#include <sys/socket.h>
#include <unistd.h>

static void Open(struct SolidSyslogDatagram* self);
static bool SendTo(struct SolidSyslogDatagram* self, const void* buffer, size_t size, const struct sockaddr_in* addr);
static void Close(struct SolidSyslogDatagram* self);

struct SolidSyslogPosixDatagram
{
    struct SolidSyslogDatagram base;
    int                        fd;
};

static struct SolidSyslogPosixDatagram instance = {.fd = -1};

struct SolidSyslogDatagram* SolidSyslogPosixDatagram_Create(void)
{
    instance.base.Open   = Open;
    instance.base.SendTo = SendTo;
    instance.base.Close  = Close;
    return &instance.base;
}

void SolidSyslogPosixDatagram_Destroy(void)
{
    instance.base.Open   = NULL;
    instance.base.SendTo = NULL;
    instance.base.Close  = NULL;
}

static void Open(struct SolidSyslogDatagram* self)
{
    struct SolidSyslogPosixDatagram* datagram = (struct SolidSyslogPosixDatagram*) self;
    datagram->fd                              = socket(AF_INET, SOCK_DGRAM, 0);
}

static bool SendTo(struct SolidSyslogDatagram* self, const void* buffer, size_t size, const struct sockaddr_in* addr)
{
    struct SolidSyslogPosixDatagram* datagram = (struct SolidSyslogPosixDatagram*) self;
    return sendto(datagram->fd, buffer, size, 0, (const struct sockaddr*) addr, sizeof(*addr)) >= 0;
}

static void Close(struct SolidSyslogDatagram* self)
{
    struct SolidSyslogPosixDatagram* datagram = (struct SolidSyslogPosixDatagram*) self;
    if (datagram->fd >= 0)
    {
        close(datagram->fd);
        datagram->fd = -1;
    }
}
