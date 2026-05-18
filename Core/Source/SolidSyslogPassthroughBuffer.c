#include "SolidSyslogPassthroughBuffer.h"

#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogBufferDefinition.h"
#include "SolidSyslogSender.h"

static bool PassthroughBuffer_Read(struct SolidSyslogBuffer* base, void* data, size_t maxSize, size_t* bytesRead);
static void PassthroughBuffer_Write(struct SolidSyslogBuffer* base, const void* data, size_t size);

static inline struct SolidSyslogPassthroughBuffer* PassthroughBuffer_SelfFromBase(struct SolidSyslogBuffer* base);

struct SolidSyslogPassthroughBuffer
{
    struct SolidSyslogBuffer Base;
    struct SolidSyslogSender* Sender;
};

static struct SolidSyslogPassthroughBuffer PassthroughBuffer_Instance;

struct SolidSyslogBuffer* SolidSyslogPassthroughBuffer_Create(struct SolidSyslogSender* sender)
{
    PassthroughBuffer_Instance.Base.Write = PassthroughBuffer_Write;
    PassthroughBuffer_Instance.Base.Read = PassthroughBuffer_Read;
    PassthroughBuffer_Instance.Sender = sender;
    return &PassthroughBuffer_Instance.Base;
}

void SolidSyslogPassthroughBuffer_Destroy(void)
{
    PassthroughBuffer_Instance.Base.Write = NULL;
    PassthroughBuffer_Instance.Base.Read = NULL;
    PassthroughBuffer_Instance.Sender = NULL;
}

static bool PassthroughBuffer_Read(struct SolidSyslogBuffer* base, void* data, size_t maxSize, size_t* bytesRead)
{
    (void) base;
    (void) data;
    (void) maxSize;
    *bytesRead = 0;
    return false;
}

static void PassthroughBuffer_Write(struct SolidSyslogBuffer* base, const void* data, size_t size)
{
    struct SolidSyslogPassthroughBuffer* self = PassthroughBuffer_SelfFromBase(base);
    (void) SolidSyslogSender_Send(self->Sender, data, size);
}

static inline struct SolidSyslogPassthroughBuffer* PassthroughBuffer_SelfFromBase(struct SolidSyslogBuffer* base)
{
    return (struct SolidSyslogPassthroughBuffer*) base;
}
