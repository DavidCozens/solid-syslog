#include "PosixUdpSender.h"

struct SolidSyslog_Sender* PosixUdpSender_Create(const struct PosixUdpSender_Config* config)
{
    (void) config;
    return NULL;
}

void PosixUdpSender_Destroy(struct SolidSyslog_Sender* sender)
{
    (void) sender;
}
