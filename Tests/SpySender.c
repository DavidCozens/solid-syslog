#include "SpySender.h"

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

void SpySender_Reset(void)
{
    callCount  = 0;
    lastBuffer = NULL;
}

const char* SpySender_LastBufferAsString(void)
{
    return (const char*) lastBuffer;
}

struct SolidSyslog_Sender* SpySender_GetSender(void)
{
    sender.Send = Send;
    return &sender;
}

int SpySender_CallCount(void)
{
    return callCount;
}
