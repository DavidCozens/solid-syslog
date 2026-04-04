#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslogBufferDef.h"

#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead);
static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size);

struct SolidSyslogPosixMqBuffer
{
    struct SolidSyslogBuffer base;
    mqd_t                    mq;
    char                     name[64];
    size_t                   maxMessageSize;
};

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- distinct semantic meaning; struct wrapper would over-engineer
struct SolidSyslogBuffer* SolidSyslogPosixMqBuffer_Create(size_t maxMessageSize, long maxMessages)
{
    struct SolidSyslogPosixMqBuffer* self = calloc(1, sizeof(struct SolidSyslogPosixMqBuffer));

    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- snprintf with bounded size; snprintf_s is not portable
    snprintf(self->name, sizeof(self->name), "/solidsyslog_%d", getpid());

    struct mq_attr attr = {0};
    attr.mq_maxmsg      = maxMessages;
    attr.mq_msgsize     = (long) maxMessageSize;

    self->mq             = mq_open(self->name, O_CREAT | O_RDWR | O_NONBLOCK, 0600, &attr);
    self->maxMessageSize = maxMessageSize;
    self->base.Write     = Write;
    self->base.Read      = Read;

    return &self->base;
}

void SolidSyslogPosixMqBuffer_Destroy(struct SolidSyslogBuffer* buffer)
{
    struct SolidSyslogPosixMqBuffer* self = (struct SolidSyslogPosixMqBuffer*) buffer;
    mq_close(self->mq);
    mq_unlink(self->name);
    free(self);
}

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead)
{
    struct SolidSyslogPosixMqBuffer* mqBuffer = (struct SolidSyslogPosixMqBuffer*) self;
    ssize_t                          received = mq_receive(mqBuffer->mq, data, maxSize, NULL);

    if (received < 0)
    {
        return false;
    }

    *bytesRead = (size_t) received;
    return true;
}

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size)
{
    struct SolidSyslogPosixMqBuffer* mqBuffer = (struct SolidSyslogPosixMqBuffer*) self;
    mq_send(mqBuffer->mq, data, size, 0);
}
