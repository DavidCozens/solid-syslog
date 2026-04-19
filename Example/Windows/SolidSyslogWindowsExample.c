#include "SolidSyslogWindowsExample.h"
#include "ExampleAppName.h"
#include "ExampleInteractive.h"
#include "ExampleWindowsCommandLine.h"
#include "SolidSyslog.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogEndpoint.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogNullStore.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogUdpSender.h"
#include "SolidSyslogWindowsClock.h"
#include "SolidSyslogWindowsHostname.h"
#include "SolidSyslogWindowsProcessId.h"
#include "SolidSyslogWinsockDatagram.h"
#include "SolidSyslogWinsockResolver.h"

#include <stdint.h>
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

static void GetEndpoint(struct SolidSyslogEndpoint* endpoint)
{
    SolidSyslogFormatter_BoundedString(endpoint->host, GetHost(), SOLIDSYSLOG_MAX_HOST_SIZE);
    endpoint->port = (uint16_t) GetPort();
}

static uint32_t GetEndpointVersion(void)
{
    return 0;
}

static void GetTimeQuality(struct SolidSyslogTimeQuality* timeQuality)
{
    timeQuality->tzKnown                  = true;
    timeQuality->isSynced                 = true;
    timeQuality->syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
}

int SolidSyslogWindowsExample_Run(int argc, char* argv[])
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return 1;
    }

    ExampleAppName_Set(argv[0]);

    struct WindowsExampleOptions options;
    ExampleWindowsCommandLine_Parse(argc, argv, &options);

    struct SolidSyslogResolver*       resolver  = SolidSyslogWinsockResolver_Create();
    struct SolidSyslogDatagram*       datagram  = SolidSyslogWinsockDatagram_Create();
    struct SolidSyslogUdpSenderConfig udpConfig = {.resolver = resolver, .datagram = datagram, .endpoint = GetEndpoint, .endpointVersion = GetEndpointVersion};
    struct SolidSyslogSender*         sender    = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer*         buffer    = SolidSyslogNullBuffer_Create(sender);
    struct SolidSyslogStore*          store     = SolidSyslogNullStore_Create();
    struct SolidSyslogAtomicCounter*  counter   = SolidSyslogAtomicCounter_Create();
    struct SolidSyslogStructuredData* metaSd    = SolidSyslogMetaSd_Create(counter);
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
        .facility  = options.facility,
        .severity  = options.severity,
        .messageId = options.messageId,
        .msg       = options.msg,
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
