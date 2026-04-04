#include "SolidSyslogExample.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogNullBuffer.h"
#include "SolidSyslogPosixClock.h"
#include "SolidSyslogPosixHostname.h"
#include "SolidSyslogPosixProcId.h"
#include "SolidSyslogUdpSender.h"

#include <getopt.h>
#include <stdlib.h>
#include <string.h>

static const char* appName;

static size_t GetAppName(char* buffer, size_t size)
{
    size_t len = strlen(appName);
    if (len >= size)
    {
        len = size - 1;
    }
    // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) -- memcpy with bounded len; memcpy_s is not portable
    memcpy(buffer, appName, len);
    buffer[len] = '\0';
    return len;
}

static const char* GetHost(void)
{
    return "syslog-ng";
}

static int GetPort(void)
{
    return 5514;
}

int SolidSyslogExample_Run(int argc, char* argv[])
{
    const char* slash = strrchr(argv[0], '/');
    appName           = (slash != NULL) ? slash + 1 : argv[0];

    enum SolidSyslog_Facility facility  = SOLIDSYSLOG_FACILITY_LOCAL0;
    enum SolidSyslog_Severity severity  = SOLIDSYSLOG_SEVERITY_INFO;
    const char*               messageId = NULL;
    const char*               msg       = NULL;

    static struct option longOptions[] = {
        {"facility", required_argument, NULL, 'f'},
        {"severity", required_argument, NULL, 's'},
        {"msgid", required_argument, NULL, 'i'},
        {"message", required_argument, NULL, 'm'},
        {NULL, 0, NULL, 0},
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "f:s:i:m:", longOptions, NULL)) != -1)
    {
        switch (opt)
        {
            case 'f':
                facility = (enum SolidSyslog_Facility) atoi(optarg);
                break;
            case 's':
                severity = (enum SolidSyslog_Severity) atoi(optarg);
                break;
            case 'i':
                messageId = optarg;
                break;
            case 'm':
                msg = optarg;
                break;
            default:
                return 1;
        }
    }

    struct SolidSyslogUdpSenderConfig udpConfig = {
        .getPort = GetPort,
        .getHost = GetHost,
    };
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer* buffer = SolidSyslogNullBuffer_Create(sender);

    struct SolidSyslogConfig config = {
        .buffer      = buffer,
        .sender      = NULL,
        .alloc       = malloc,
        .free        = free,
        .clock       = SolidSyslogPosixClock_GetTimestamp,
        .getHostname = SolidSyslogPosixHostname_Get,
        .getAppName  = GetAppName,
        .getProcId   = SolidSyslogPosixProcId_Get,
    };
    struct SolidSyslog* logger = SolidSyslog_Create(&config);

    struct SolidSyslogMessage message = {
        .facility  = facility,
        .severity  = severity,
        .messageId = messageId,
        .msg       = msg,
    };
    SolidSyslog_Log(logger, &message);

    SolidSyslog_Destroy(logger);
    SolidSyslogNullBuffer_Destroy(buffer);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
