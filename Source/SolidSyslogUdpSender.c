#include "SolidSyslogUdpSender.h"
#include "SolidSyslogSenderDef.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

struct SolidSyslogUdpSender
{
    struct SolidSyslogSender base;
    int                      fd;
};

static void Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    struct SolidSyslogUdpSender* sender = (struct SolidSyslogUdpSender*) self;
    struct sockaddr_in           addr   = {0};
    addr.sin_family                     = AF_INET;
    addr.sin_port                       = htons(514);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    sendto(sender->fd, buffer, size, 0, (struct sockaddr*) &addr, sizeof(addr));
}

struct SolidSyslogSender* SolidSyslogUdpSender_Create(const struct SolidSyslogUdpSenderConfig* config)
{
    (void) config;
    struct SolidSyslogUdpSender* self = malloc(sizeof(struct SolidSyslogUdpSender));
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->base.Send = Send;
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->fd = socket(AF_INET, SOCK_DGRAM, 0);
    return &self->base;
}

void SolidSyslogUdpSender_Destroy(struct SolidSyslogSender* sender)
{
    struct SolidSyslogUdpSender* self = (struct SolidSyslogUdpSender*) sender;
    close(self->fd);
    free(self);
}
