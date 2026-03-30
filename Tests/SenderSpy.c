#include "SenderSpy.h"
#include "SolidSyslogSenderDef.h"

static int                      callCount;
static const void*              lastBuffer;
static struct SolidSyslogSender sender;

static void Send(struct SolidSyslogSender* self, const void* buffer, size_t size)
{
    (void) self;
    (void) size;
    callCount++;
    lastBuffer = buffer;
}

void SenderSpy_Reset(void)
{
    callCount  = 0;
    lastBuffer = NULL;
}

const char* SenderSpy_LastBufferAsString(void)
{
    return (const char*) lastBuffer;
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
