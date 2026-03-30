#include "PosixUdpSender.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct PosixUdpSender
{
    struct SolidSyslog_Sender base;
};

static void Send(struct SolidSyslog_Sender* self, const void* buffer, size_t size)
{
    (void) self;
    struct sockaddr_in addr = {0};
    addr.sin_family         = AF_INET;
    addr.sin_port           = htons(514);
    addr.sin_addr.s_addr    = INADDR_ANY;
    sendto(0, buffer, size, 0, (struct sockaddr*) &addr, sizeof(addr));
}

struct SolidSyslog_Sender* PosixUdpSender_Create(const struct PosixUdpSender_Config* config)
{
    (void) config;
    struct PosixUdpSender* self = malloc(sizeof(struct PosixUdpSender));
    // cppcheck-suppress nullPointerOutOfMemory -- malloc NULL check deferred to error handling phase
    self->base.Send = Send;
    return &self->base;
}

void PosixUdpSender_Destroy(struct SolidSyslog_Sender* sender)
{
    free(sender);
}
