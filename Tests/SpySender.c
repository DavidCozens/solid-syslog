#include "SpySender.h"

static int callCount;
static struct SolidSyslog_Sender sender;

static void Send(struct SolidSyslog_Sender *self, const void *buffer, size_t size)
{
    (void)self;
    (void)buffer;
    (void)size;
    callCount++;
}

void SpySender_Reset(void)
{
    callCount = 0;
}

struct SolidSyslog_Sender *SpySender_GetSender(void)
{
    sender.Send = Send;
    return &sender;
}

int SpySender_CallCount(void)
{
    return callCount;
}
