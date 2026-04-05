#include "SolidSyslogExample.h"
#include "ExampleAppName.h"
#include "ExampleCommandLine.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogPosixClock.h"
#include "SolidSyslogPosixHostname.h"
#include "SolidSyslogPosixProcId.h"
#include "SolidSyslogUdpSender.h"

#include <stdlib.h>

int SolidSyslogExample_Run(int argc, char* argv[])
{
    ExampleAppName_Set(argv[0]);

    struct ExampleOptions options;
    if (ExampleCommandLine_Parse(argc, argv, &options) != 0)
    {
        return 1;
    }

    struct SolidSyslogUdpSenderConfig udpConfig = {
        .getPort = ExampleUdpConfig_GetPort,
        .getHost = ExampleUdpConfig_GetHost,
    };
    struct SolidSyslogSender*         sender = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer*         buffer = SolidSyslogNullBuffer_Create(sender);
    struct SolidSyslogStructuredData* metaSd = SolidSyslogMetaSd_Create(malloc);

    struct SolidSyslogConfig config = {
        .buffer      = buffer,
        .sender      = NULL,
        .alloc       = malloc,
        .free        = free,
        .clock       = SolidSyslogPosixClock_GetTimestamp,
        .getHostname = SolidSyslogPosixHostname_Get,
        .getAppName  = ExampleAppName_Get,
        .getProcId   = SolidSyslogPosixProcId_Get,
        .sd          = metaSd,
    };
    struct SolidSyslog* logger = SolidSyslog_Create(&config);

    struct SolidSyslogMessage message = {
        .facility  = options.facility,
        .severity  = options.severity,
        .messageId = options.messageId,
        .msg       = options.msg,
    };
    for (int i = 0; i < options.count; i++)
    {
        SolidSyslog_Log(logger, &message);
    }

    SolidSyslog_Destroy(logger);
    SolidSyslogMetaSd_Destroy(metaSd, free);
    SolidSyslogNullBuffer_Destroy(buffer);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
