#include "ExampleAppName.h"
#include "ExampleCommandLine.h"
#include "ExampleServiceThread.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogPosixClock.h"
#include "SolidSyslogPosixHostname.h"
#include "SolidSyslogPosixMqBuffer.h"
#include "SolidSyslogPosixProcId.h"
#include "SolidSyslogUdpSender.h"

#include <pthread.h>
#include <stdlib.h>

static volatile bool shutdown_flag;

struct ServiceThreadArgs
{
    struct SolidSyslog* logger;
    volatile bool*      shutdown;
};

static void* ServiceThreadEntry(void* arg)
{
    struct ServiceThreadArgs* args = (struct ServiceThreadArgs*) arg;
    ExampleServiceThread_Run(args->logger, args->shutdown);
    return NULL;
}

int main(int argc, char* argv[])
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
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&udpConfig);
    struct SolidSyslogBuffer* buffer = SolidSyslogPosixMqBuffer_Create(SOLIDSYSLOG_MAX_MESSAGE_SIZE, 10);

    struct SolidSyslogConfig config = {
        .buffer      = buffer,
        .sender      = sender,
        .alloc       = malloc,
        .free        = free,
        .clock       = SolidSyslogPosixClock_GetTimestamp,
        .getHostname = SolidSyslogPosixHostname_Get,
        .getAppName  = ExampleAppName_Get,
        .getProcId   = SolidSyslogPosixProcId_Get,
    };
    struct SolidSyslog* logger = SolidSyslog_Create(&config);

    shutdown_flag = false;

    struct ServiceThreadArgs threadArgs = {logger, &shutdown_flag};
    pthread_t                serviceThread;
    pthread_create(&serviceThread, NULL, ServiceThreadEntry, &threadArgs);

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

    shutdown_flag = true;
    pthread_join(serviceThread, NULL);

    SolidSyslog_Destroy(logger);
    SolidSyslogPosixMqBuffer_Destroy(buffer);
    SolidSyslogUdpSender_Destroy(sender);

    return 0;
}
