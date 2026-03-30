#include "SolidSyslogSenderDef.h"

void SolidSyslogSender_Send(struct SolidSyslogSender* sender, const void* buffer, size_t size)
{
    sender->Send(sender, buffer, size);
}
