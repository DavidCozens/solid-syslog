#include "SolidSyslog_Sender_Impl.h"

void SolidSyslog_Sender_Send(struct SolidSyslog_Sender* sender, const void* buffer, size_t size)
{
    sender->Send(sender, buffer, size);
}
