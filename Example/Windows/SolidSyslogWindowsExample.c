#include "SolidSyslogWindowsExample.h"
#include "ExampleAppName.h"
#include "ExampleInteractive.h"
#include "SolidSyslog.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogNullStore.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogUdpSender.h"
#include "SolidSyslogWindowsClock.h"
#include "SolidSyslogWindowsHostname.h"
#include "SolidSyslogWindowsProcessId.h"
#include "SolidSyslogWinsockDatagram.h"
#include "SolidSyslogWinsockResolver.h"

#include <stdio.h>
#include <winsock2.h>

enum
{
    EXAMPLE_UDP_PORT = 5514
};

static const char* GetHost(void)
{
    return "127.0.0.1";
}

static int GetPort(void)
{
    return EXAMPLE_UDP_PORT;
}

static void GetTimeQuality(struct SolidSyslogTimeQuality* timeQuality)
{
    timeQuality->tzKnown                  = true;
    timeQuality->isSynced                 = true;
    timeQuality->syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
}

int SolidSyslogWindowsExample_Run(int argc, char* argv[])
{
    (void) argc;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    ExampleAppName_Set(argv[0]);

    struct SolidSyslogResolver*       resolver    = SolidSyslogWinsockResolver_Create(GetHost, GetPort);
    struct SolidSyslogDatagram*       datagram    = SolidSyslogWinsockDatagram_Create();
    struct SolidSyslogUdpSenderConfig udpConfig   = {.resolver = resolver, .datagram = datagram};
    struct SolidSyslogSender*         sender      = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer*         buffer      = SolidSyslogNullBuffer_Create(sender);
    struct SolidSyslogStore*          store       = SolidSyslogNullStore_Create();
    struct SolidSyslogAtomicCounter*  counter     = SolidSyslogAtomicCounter_Create();
    struct SolidSyslogStructuredData* metaSd      = SolidSyslogMetaSd_Create(counter);
    struct SolidSyslogStructuredData* timeQuality = SolidSyslogTimeQualitySd_Create(GetTimeQuality);
    struct SolidSyslogStructuredData* originSd    = SolidSyslogOriginSd_Create("SolidSyslogExample", "0.7.0");

    struct SolidSyslogStructuredData* sdList[] = {metaSd, timeQuality, originSd};

    struct SolidSyslogConfig config = {
        .buffer       = buffer,
        .sender       = NULL,
        .clock        = SolidSyslogWindowsClock_GetTimestamp,
        .getHostname  = SolidSyslogWindowsHostname_Get,
        .getAppName   = ExampleAppName_Get,
        .getProcessId = SolidSyslogWindowsProcessId_Get,
        .store        = store,
        .sd           = sdList,
        .sdCount      = sizeof(sdList) / sizeof(sdList[0]),
    };
    SolidSyslog_Create(&config);

    struct SolidSyslogMessage message = {
        .facility  = SOLIDSYSLOG_FACILITY_LOCAL0,
        .severity  = SOLIDSYSLOG_SEVERITY_INFO,
        .messageId = NULL,
        .msg       = NULL,
    };

    ExampleInteractive_Run(&message, stdin);

    SolidSyslog_Destroy();
    SolidSyslogOriginSd_Destroy();
    SolidSyslogTimeQualitySd_Destroy();
    SolidSyslogMetaSd_Destroy();
    SolidSyslogAtomicCounter_Destroy();
    SolidSyslogNullStore_Destroy();
    SolidSyslogNullBuffer_Destroy();
    SolidSyslogUdpSender_Destroy();
    SolidSyslogWinsockDatagram_Destroy();
    SolidSyslogWinsockResolver_Destroy();

    WSACleanup();

    return 0;
}
