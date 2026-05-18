#include "SolidSyslogNullSender.h"

#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogSenderDefinition.h"

static bool NullSender_Send(struct SolidSyslogSender* base, const void* buffer, size_t size);
static void NullSender_Disconnect(struct SolidSyslogSender* base);

static struct SolidSyslogSender instance = {.Send = NullSender_Send, .Disconnect = NullSender_Disconnect};

struct SolidSyslogSender* SolidSyslogNullSender_Get(void)
{
    return &instance;
}

static bool NullSender_Send(struct SolidSyslogSender* base, const void* buffer, size_t size)
{
    (void) base;
    (void) buffer;
    (void) size;
    return false;
}

static void NullSender_Disconnect(struct SolidSyslogSender* base)
{
    (void) base;
}
