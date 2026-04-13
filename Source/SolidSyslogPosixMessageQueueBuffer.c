#include "SolidSyslogPosixMessageQueueBuffer.h"
#include "SolidSyslogBufferDefinition.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogPosixProcessId.h"

#include <mqueue.h>
#include <time.h>

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead);
static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size);

struct SolidSyslogPosixMessageQueueBuffer
{
    struct SolidSyslogBuffer base;
    mqd_t                    mq;
    char                     name[64];
    size_t                   maxMessageSize;
};

static struct SolidSyslogPosixMessageQueueBuffer instance;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- distinct semantic meaning; struct wrapper would over-engineer
struct SolidSyslogBuffer* SolidSyslogPosixMessageQueueBuffer_Create(size_t maxMessageSize, long maxMessages)
{
    instance = (struct SolidSyslogPosixMessageQueueBuffer) {0};

    struct SolidSyslogFormatter nameFormatter;
    SolidSyslogFormatter_Create(&nameFormatter, instance.name, sizeof(instance.name));
    SolidSyslogFormatter_BoundedString(&nameFormatter, "/solidsyslog_", 13);
    SolidSyslogFormatter_Callback(&nameFormatter, SolidSyslogPosixProcessId_Get, SolidSyslogFormatter_Remaining(&nameFormatter));

    struct mq_attr attr = {0};
    attr.mq_maxmsg      = maxMessages;
    attr.mq_msgsize     = (long) maxMessageSize;

    instance.mq             = mq_open(instance.name, O_CREAT | O_RDWR | O_NONBLOCK, 0600, &attr);
    instance.maxMessageSize = maxMessageSize;
    instance.base.Write     = Write;
    instance.base.Read      = Read;

    return &instance.base;
}

void SolidSyslogPosixMessageQueueBuffer_Destroy(void)
{
    mq_close(instance.mq);
    mq_unlink(instance.name);
    instance = (struct SolidSyslogPosixMessageQueueBuffer) {0};
}

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead)
{
    struct SolidSyslogPosixMessageQueueBuffer* mqBuffer = (struct SolidSyslogPosixMessageQueueBuffer*) self;
    ssize_t                                    received = mq_receive(mqBuffer->mq, data, maxSize, NULL);
    bool                                       success  = received >= 0;

    *bytesRead = success ? (size_t) received : 0;

    return success;
}

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size)
{
    struct SolidSyslogPosixMessageQueueBuffer* mqBuffer = (struct SolidSyslogPosixMessageQueueBuffer*) self;
    mq_send(mqBuffer->mq, data, size, 0);
}
