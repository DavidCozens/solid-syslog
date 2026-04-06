#include "SolidSyslogExample.h"
#include "ExampleAppName.h"
#include "ExampleCommandLine.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogPosixClock.h"
#include "SolidSyslogPosixHostname.h"
#include "SolidSyslogPosixProcId.h"
#include "SolidSyslogUdpSender.h"

#include <stdlib.h>

static void GetTimeQuality(struct SolidSyslogTimeQuality* timeQuality)
{
    timeQuality->tzKnown                  = true;
    timeQuality->isSynced                 = true;
    timeQuality->syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
}

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
    struct SolidSyslogSender*         sender      = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer*         buffer      = SolidSyslogNullBuffer_Create(sender);
    struct SolidSyslogAtomicCounter*  counter     = SolidSyslogAtomicCounter_Create(malloc);
    struct SolidSyslogStructuredData* metaSd      = SolidSyslogMetaSd_Create(malloc, counter);
    struct SolidSyslogStructuredData* timeQuality = SolidSyslogTimeQualitySd_Create(malloc, GetTimeQuality);
    struct SolidSyslogStructuredData* originSd    = SolidSyslogOriginSd_Create(malloc, "SolidSyslogExample", "0.7.0");

    struct SolidSyslogStructuredData* sdList[] = {metaSd, timeQuality, originSd};

    struct SolidSyslogConfig config = {
        .buffer      = buffer,
        .sender      = NULL,
        .clock       = SolidSyslogPosixClock_GetTimestamp,
        .getHostname = SolidSyslogPosixHostname_Get,
        .getAppName  = ExampleAppName_Get,
        .getProcId   = SolidSyslogPosixProcId_Get,
        .sd          = sdList,
        .sdCount     = sizeof(sdList) / sizeof(sdList[0]),
    };
    SolidSyslog_Create(&config);

    struct SolidSyslogMessage message = {
        .facility  = options.facility,
        .severity  = options.severity,
        .messageId = options.messageId,
        .msg       = options.msg,
    };
    for (int i = 0; i < options.count; i++)
    {
        SolidSyslog_Log(&message);
    }

    SolidSyslog_Destroy();
    SolidSyslogOriginSd_Destroy(originSd, free);
    SolidSyslogTimeQualitySd_Destroy(timeQuality, free);
    SolidSyslogMetaSd_Destroy(metaSd, free);
    SolidSyslogAtomicCounter_Destroy(counter, free);
    SolidSyslogNullBuffer_Destroy(buffer);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
