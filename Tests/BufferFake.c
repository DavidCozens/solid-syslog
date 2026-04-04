#include "BufferFake.h"
#include "SolidSyslogBufferDef.h"

#include <stdlib.h>
#include <string.h>

enum
{
    BUFFERFAKE_MAX_SIZE = 1024
};

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead);
static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size);

struct BufferFake
{
    struct SolidSyslogBuffer base;
    char                     stored[BUFFERFAKE_MAX_SIZE];
    size_t                   storedSize;
    bool                     pending;
};

struct SolidSyslogBuffer* BufferFake_Create(void)
{
    struct BufferFake* self = calloc(1, sizeof(struct BufferFake));
    self->base.Write        = Write;
    self->base.Read         = Read;
    return &self->base;
}

void BufferFake_Destroy(struct SolidSyslogBuffer* buffer)
{
    free(buffer);
}

static bool Read(struct SolidSyslogBuffer* self, void* data, size_t maxSize, size_t* bytesRead)
{
    struct BufferFake* fake = (struct BufferFake*) self;

    if (!fake->pending)
    {
        return false;
    }

    size_t copySize = fake->storedSize < maxSize ? fake->storedSize : maxSize;
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded copySize; memcpy_s is not portable
    memcpy(data, fake->stored, copySize);
    *bytesRead    = copySize;
    fake->pending = false;

    return true;
}

static void Write(struct SolidSyslogBuffer* self, const void* data, size_t size)
{
    struct BufferFake* fake = (struct BufferFake*) self;

    size_t copySize = size < sizeof(fake->stored) ? size : sizeof(fake->stored);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded copySize; memcpy_s is not portable
    memcpy(fake->stored, data, copySize);
    fake->storedSize = copySize;
    fake->pending    = true;
}
