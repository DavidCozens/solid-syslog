#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslogBufferDef.h"
#include "SolidSyslogFormat.h"

#include <mqueue.h>
#include <stdlib.h>
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

    size_t nameLen = 0;
    nameLen += SolidSyslogFormat_BoundedString(self->name + nameLen, "/solidsyslog_", sizeof(self->name) - nameLen);
    nameLen += SolidSyslogFormat_Uint32(self->name + nameLen, (uint32_t) getpid());

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
    bool                             success  = received >= 0;

    *bytesRead = success ? (size_t) received : 0;

    return success;
}

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size)
{
    struct SolidSyslogPosixMqBuffer* mqBuffer = (struct SolidSyslogPosixMqBuffer*) self;
    mq_send(mqBuffer->mq, data, size, 0);
}
