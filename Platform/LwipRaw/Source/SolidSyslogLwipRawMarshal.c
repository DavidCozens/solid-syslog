#include "SolidSyslogLwipRawMarshalPrivate.h"

#include <stddef.h>

static void Marshal_NullMarshal(SolidSyslogLwipRawCallback callback, void* context)
{
    callback(context);
}

static SolidSyslogLwipRawMarshalFunction currentMarshal = Marshal_NullMarshal;

void SolidSyslogLwipRaw_SetMarshal(SolidSyslogLwipRawMarshalFunction marshal)
{
    if (marshal == NULL)
    {
        currentMarshal = Marshal_NullMarshal;
    }
    else
    {
        currentMarshal = marshal;
    }
}

void SolidSyslogLwipRaw_Marshal(SolidSyslogLwipRawCallback callback, void* context)
{
    currentMarshal(callback, context);
}
