#include "SenderFake.h"
#include "SolidSyslogSenderDefinition.h"

#include <string.h>

static inline size_t MinSize(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

enum
{
    SENDERFAKE_MAX_BUFFER_SIZE = 1024
};

static int                      callCount;
static char                     lastBuffer[SENDERFAKE_MAX_BUFFER_SIZE];
static size_t                   lastSize;
static bool                     failNextSend;
static struct SolidSyslogSender sender;

static bool Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    (void) self;
    size_t copySize = MinSize(size, sizeof(lastBuffer) - 1);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded copySize; memcpy_s is not portable
    memcpy(lastBuffer, buffer, copySize);
    lastBuffer[copySize] = '\0';
    lastSize             = size;
    callCount++;

    if (failNextSend)
    {
        failNextSend = false;
        return false;
    }
    return true;
}

void SenderFake_Reset(void)
{
    callCount     = 0;
    lastBuffer[0] = '\0';
    lastSize      = 0;
    failNextSend  = false;
}

void SenderFake_FailNextSend(void)
{
    failNextSend = true;
}

const char* SenderFake_LastBufferAsString(void)
{
    return lastBuffer;
}

struct SolidSyslogSender* SenderFake_GetSender(void)
{
    sender.Send = Send;
    return &sender;
}

int SenderFake_CallCount(void)
{
    return callCount;
}

size_t SenderFake_LastSize(void)
{
    return lastSize;
}
