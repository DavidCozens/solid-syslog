#include "SolidSyslog.h"
#include "SolidSyslogUdpSender.h"

#include <stdlib.h>

static const char* GetHost(void)
{
    return "syslog-ng";
}

static int GetPort(void)
{
    return 5514;
}

int main(void)
{
    struct SolidSyslogUdpSenderConfig udpConfig = {
        .getPort = GetPort,
        .getHost = GetHost,
    };
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&udpConfig);

    struct SolidSyslogConfig config = {
        .sender = sender,
        .alloc  = malloc,
        .free   = free,
    };
    struct SolidSyslog* logger = SolidSyslog_Create(&config);

    SolidSyslog_Log(logger);

    SolidSyslog_Destroy(logger);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
