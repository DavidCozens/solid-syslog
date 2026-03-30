#include "PosixUdpSender.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

struct PosixUdpSender
{
    struct SolidSyslog_Sender base;
    int                       fd;
};

static void Send(struct SolidSyslog_Sender* self, const void* buffer, size_t size)
{
    struct PosixUdpSender* sender = (struct PosixUdpSender*) self;
    struct sockaddr_in     addr   = {0};
    addr.sin_family               = AF_INET;
    addr.sin_port                 = htons(514);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    sendto(sender->fd, buffer, size, 0, (struct sockaddr*) &addr, sizeof(addr));
}

struct SolidSyslog_Sender* PosixUdpSender_Create(const struct PosixUdpSender_Config* config)
{
    (void) config;
    struct PosixUdpSender* self = malloc(sizeof(struct PosixUdpSender));
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->base.Send = Send;
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->fd = socket(AF_INET, SOCK_DGRAM, 0);
    return &self->base;
}

void PosixUdpSender_Destroy(struct SolidSyslog_Sender* sender)
{
    struct PosixUdpSender* self = (struct PosixUdpSender*) sender;
    close(self->fd);
    free(self);
}
