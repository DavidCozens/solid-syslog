#include "SenderSpy.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogSenderDef.h"

#include <string.h>

enum
{
    SENDERSPY_MAX_BUFFER_SIZE = 1024
};

static int                      callCount;
static char                     lastBuffer[SENDERSPY_MAX_BUFFER_SIZE];
static size_t                   lastSize;
static struct SolidSyslogSender sender;

static void Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    (void) self;
    size_t copySize = SolidSyslogFormat_MinSize(size, sizeof(lastBuffer) - 1);
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded copySize; memcpy_s is not portable
    memcpy(lastBuffer, buffer, copySize);
    lastBuffer[copySize] = '\0';
    lastSize             = size;
    callCount++;
}

void SenderSpy_Reset(void)
{
    callCount     = 0;
    lastBuffer[0] = '\0';
    lastSize      = 0;
}

const char* SenderSpy_LastBufferAsString(void)
{
    return lastBuffer;
}

struct SolidSyslogSender* SenderSpy_GetSender(void)
{
    sender.Send = Send;
    return &sender;
}

int SenderSpy_CallCount(void)
{
    return callCount;
}

size_t SenderSpy_LastSize(void)
{
    return lastSize;
}
