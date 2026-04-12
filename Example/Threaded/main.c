#include "ExampleAppName.h"
#include "ExampleCommandLine.h"
#include "ExampleInteractive.h"
#include "ExampleServiceThread.h"
#include "ExampleTcpConfig.h"
#include "ExampleUdpConfig.h"
#include "SolidSyslog.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogCrc16Policy.h"
#include "SolidSyslogFileStore.h"
#include "SolidSyslogMetaSd.h"
#include "SolidSyslogOriginSd.h"
#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogNullStore.h"
#include "SolidSyslogPosixClock.h"
#include "SolidSyslogPosixFile.h"
#include "SolidSyslogPosixHostname.h"
#include "SolidSyslogPosixMessageQueueBuffer.h"
#include "SolidSyslogPosixProcessId.h"
#include "SolidSyslogTcpSender.h"
#include "SolidSyslogUdpSender.h"

#include <pthread.h>
#include <string.h>
#include <unistd.h>

static const char* const       STORE_PATH_PREFIX = "/tmp/STORE";
static struct SolidSyslogFile* storeFile;

enum
{
    DEFAULT_MAX_FILE_SIZE = 65536,
    DEFAULT_MAX_FILES     = 10
};

static void GetTimeQuality(struct SolidSyslogTimeQuality* timeQuality)
{
    timeQuality->tzKnown                  = true;
    timeQuality->isSynced                 = true;
    timeQuality->syncAccuracyMicroseconds = SOLIDSYSLOG_SYNC_ACCURACY_OMIT;
}

static volatile bool shutdown_flag;

static void* ServiceThreadEntry(void* arg)
{
    volatile bool* shutdown = (volatile bool*) arg;
    ExampleServiceThread_Run(shutdown);
    return NULL;
}

static struct SolidSyslogSender* CreateSender(const struct ExampleOptions* options)
{
    bool useTcp = (strcmp(options->transport, "tcp") == 0);

    if (useTcp)
    {
        static struct SolidSyslogTcpSenderConfig tcpConfig = {0};
        tcpConfig.getPort                                  = ExampleTcpConfig_GetPort;
        tcpConfig.getHost                                  = ExampleTcpConfig_GetHost;
        return SolidSyslogTcpSender_Create(&tcpConfig);
    }

    static struct SolidSyslogUdpSenderConfig udpConfig = {0};
    udpConfig.getPort                                  = ExampleUdpConfig_GetPort;
    udpConfig.getHost                                  = ExampleUdpConfig_GetHost;
    return SolidSyslogUdpSender_Create(&udpConfig);
}

static struct SolidSyslogStore* CreateStore(const struct ExampleOptions* options)
{
    bool useFile = (strcmp(options->store, "file") == 0);

    if (useFile)
    {
        static struct SolidSyslogPosixFileStorage fileStorage;
        storeFile                    = SolidSyslogPosixFile_Create(&fileStorage);
        struct SolidSyslogFile* file = storeFile;

        static struct SolidSyslogFileStoreConfig storeConfig = {0};
        storeConfig.readFile                                 = file;
        storeConfig.writeFile                                = file;
        storeConfig.pathPrefix                               = STORE_PATH_PREFIX;
        storeConfig.maxFileSize                              = DEFAULT_MAX_FILE_SIZE;
        storeConfig.maxFiles                                 = DEFAULT_MAX_FILES;
        storeConfig.discardPolicy                            = SOLIDSYSLOG_DISCARD_OLDEST;
        storeConfig.securityPolicy                           = SolidSyslogCrc16Policy_Create();
        return SolidSyslogFileStore_Create(&storeConfig);
    }

    return SolidSyslogNullStore_Create();
}

static void DestroySender(const struct ExampleOptions* options)
{
    bool useTcp = (strcmp(options->transport, "tcp") == 0);

    if (useTcp)
    {
        SolidSyslogTcpSender_Destroy();
    }
    else
    {
        SolidSyslogUdpSender_Destroy();
    }
}

static void DestroyStore(const struct ExampleOptions* options)
{
    bool useFile = (strcmp(options->store, "file") == 0);

    if (useFile)
    {
        SolidSyslogFileStore_Destroy();
        SolidSyslogCrc16Policy_Destroy();
        SolidSyslogPosixFile_Destroy(storeFile);
    }
    else
    {
        SolidSyslogNullStore_Destroy();
    }
}

int main(int argc, char* argv[])
{
    ExampleAppName_Set(argv[0]);

    struct ExampleOptions options;
    if (ExampleCommandLine_Parse(argc, argv, &options) != 0)
    {
        return 1;
    }

    struct SolidSyslogSender* sender = CreateSender(&options);
    struct SolidSyslogStore*  store  = CreateStore(&options);

    struct SolidSyslogBuffer*         buffer      = SolidSyslogPosixMessageQueueBuffer_Create(SOLIDSYSLOG_MAX_MESSAGE_SIZE, 10);
    struct SolidSyslogAtomicCounter*  counter     = SolidSyslogAtomicCounter_Create();
    struct SolidSyslogStructuredData* metaSd      = SolidSyslogMetaSd_Create(counter);
    struct SolidSyslogStructuredData* timeQuality = SolidSyslogTimeQualitySd_Create(GetTimeQuality);
    struct SolidSyslogStructuredData* originSd    = SolidSyslogOriginSd_Create("SolidSyslogExample", "0.7.0");

    struct SolidSyslogStructuredData* sdList[] = {metaSd, timeQuality, originSd};

    struct SolidSyslogConfig config = {
        .buffer       = buffer,
        .sender       = sender,
        .clock        = SolidSyslogPosixClock_GetTimestamp,
        .getHostname  = SolidSyslogPosixHostname_Get,
        .getAppName   = ExampleAppName_Get,
        .getProcessId = SolidSyslogPosixProcessId_Get,
        .store        = store,
        .sd           = sdList,
        .sdCount      = sizeof(sdList) / sizeof(sdList[0]),
    };
    SolidSyslog_Create(&config);

    shutdown_flag = false;

    pthread_t serviceThread = 0;
    pthread_create(&serviceThread, NULL, ServiceThreadEntry, (void*) &shutdown_flag);

    struct SolidSyslogMessage message = {
        .facility  = options.facility,
        .severity  = options.severity,
        .messageId = options.messageId,
        .msg       = options.msg,
    };

    ExampleInteractive_Run(&message, stdin);

    shutdown_flag = true;
    pthread_join(serviceThread, NULL);

    SolidSyslog_Destroy();
    SolidSyslogOriginSd_Destroy();
    SolidSyslogTimeQualitySd_Destroy();
    SolidSyslogMetaSd_Destroy();
    SolidSyslogAtomicCounter_Destroy();
    DestroyStore(&options);
    SolidSyslogPosixMessageQueueBuffer_Destroy();
    DestroySender(&options);

    return 0;
}
