#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogBufferDef.h"
#include "SolidSyslogSender.h"

#include <stdlib.h>

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size);

struct SolidSyslogNullBuffer
{
    struct SolidSyslogBuffer  base;
    struct SolidSyslogSender* sender;
};

struct SolidSyslogBuffer* SolidSyslogNullBuffer_Create(struct SolidSyslogSender* sender)
{
    struct SolidSyslogNullBuffer* self = malloc(sizeof(struct SolidSyslogNullBuffer));
    self->base.Write                   = Write;
    self->sender                       = sender;
    return &self->base;
}

void SolidSyslogNullBuffer_Destroy(struct SolidSyslogBuffer* buffer)
{
    free(buffer);
}

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size)
{
    struct SolidSyslogNullBuffer* nullBuffer = (struct SolidSyslogNullBuffer*) self;
    SolidSyslogSender_Send(nullBuffer->sender, data, size);
}
