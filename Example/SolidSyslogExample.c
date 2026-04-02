#include "SolidSyslog.h"
#include "SolidSyslogUdpSender.h"

#include <getopt.h>
#include <stdlib.h>

static const char* GetHost(void)
{
    return "syslog-ng";
}

static int GetPort(void)
{
    return 5514;
}

int main(int argc, char* argv[])
{
    enum SolidSyslog_Facility facility = SOLIDSYSLOG_FACILITY_LOCAL0;
    enum SolidSyslog_Severity severity = SOLIDSYSLOG_SEVERITY_INFO;

    static struct option longOptions[] = {
        {"facility", required_argument, NULL, 'f'},
        {"severity", required_argument, NULL, 's'},
        {NULL, 0, NULL, 0},
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "f:s:", longOptions, NULL)) != -1)
    {
        switch (opt)
        {
            case 'f':
                facility = (enum SolidSyslog_Facility) atoi(optarg);
                break;
            case 's':
                severity = (enum SolidSyslog_Severity) atoi(optarg);
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

    struct SolidSyslogConfig config = {
        .sender = sender,
        .alloc  = malloc,
        .free   = free,
    };
    struct SolidSyslog* logger = SolidSyslog_Create(&config);

    SolidSyslog_Log(logger, facility, severity);

    SolidSyslog_Destroy(logger);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
