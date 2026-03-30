#include "SenderSpy.h"

static int                       callCount;
static const void*               lastBuffer;
static struct SolidSyslog_Sender sender;

static void Send(struct SolidSyslog_Sender* self, const void* buffer, size_t size)
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

struct SolidSyslog_Sender* SenderSpy_GetSender(void)
{
    sender.Send = Send;
    return &sender;
}

int SenderSpy_CallCount(void)
{
    return callCount;
}
