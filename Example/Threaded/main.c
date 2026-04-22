#include "ExampleAppName.h"
#include "ExampleCommandLine.h"
#include "ExampleInteractive.h"
#include "ExampleServiceThread.h"
#include "ExampleSwitchConfig.h"
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
#include "SolidSyslogGetAddrInfoResolver.h"
#include "SolidSyslogPosixDatagram.h"
#include "SolidSyslogPosixTcpStream.h"
#include "SolidSyslogSwitchingSender.h"
#include "SolidSyslogStreamSender.h"
#include "SolidSyslogUdpSender.h"

/* TODO(S3.13, #171): replace these three #ifdef blocks with a per-backend
 * ExampleTlsSender factory selected at CMake time. */
#ifdef SOLIDSYSLOG_HAVE_OPENSSL
#include "ExampleMtlsConfig.h"
#include "ExampleTlsConfig.h"
#include "SolidSyslogTlsStream.h"
#endif

#include <pthread.h>
#include <string.h>
#include <unistd.h>

static const char* const                STORE_PATH_PREFIX = "/tmp/STORE";
static struct SolidSyslogFile*          storeReadFile;
static struct SolidSyslogFile*          storeWriteFile;
static SolidSyslogPosixTcpStreamStorage plainTcpStreamStorage;
static struct SolidSyslogStream*        plainTcpStream;
static SolidSyslogStreamSenderStorage   plainTcpSenderStorage;
static struct SolidSyslogSender*        plainTcpSender;
#ifdef SOLIDSYSLOG_HAVE_OPENSSL
static SolidSyslogPosixTcpStreamStorage tlsUnderlyingStreamStorage;
static struct SolidSyslogStream*        tlsUnderlyingStream;
static SolidSyslogTlsStreamStorage      tlsStreamStorage;
static struct SolidSyslogStream*        tlsStream;
static SolidSyslogStreamSenderStorage   tlsSenderStorage;
static struct SolidSyslogSender*        tlsSender;
#endif

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
    bool mtlsModeActive = (strcmp(options->transport, "mtls") == 0);

    struct SolidSyslogResolver* resolver = SolidSyslogGetAddrInfoResolver_Create();

    static struct SolidSyslogUdpSenderConfig udpConfig = {0};
    udpConfig.resolver                                 = resolver;
    udpConfig.datagram                                 = SolidSyslogPosixDatagram_Create();
    udpConfig.endpoint                                 = ExampleUdpConfig_GetEndpoint;
    udpConfig.endpointVersion                          = ExampleUdpConfig_GetEndpointVersion;
    struct SolidSyslogSender* udpSender                = SolidSyslogUdpSender_Create(&udpConfig);

    plainTcpStream                                        = SolidSyslogPosixTcpStream_Create(&plainTcpStreamStorage);
    static struct SolidSyslogStreamSenderConfig tcpConfig = {0};
    tcpConfig.resolver                                    = resolver;
    tcpConfig.stream                                      = plainTcpStream;
    tcpConfig.endpoint                                    = ExampleTcpConfig_GetEndpoint;
    tcpConfig.endpointVersion                             = ExampleTcpConfig_GetEndpointVersion;
    plainTcpSender                                        = SolidSyslogStreamSender_Create(&plainTcpSenderStorage, &tcpConfig);

    struct SolidSyslogSender* tlsSlot = NULL;
#ifdef SOLIDSYSLOG_HAVE_OPENSSL
    tlsUnderlyingStream                                      = SolidSyslogPosixTcpStream_Create(&tlsUnderlyingStreamStorage);
    static struct SolidSyslogTlsStreamConfig tlsStreamConfig = {0};
    tlsStreamConfig.transport                                = tlsUnderlyingStream;
    if (mtlsModeActive)
    {
        tlsStreamConfig.caBundlePath        = ExampleMtlsConfig_GetCaBundlePath();
        tlsStreamConfig.serverName          = ExampleMtlsConfig_GetServerName();
        tlsStreamConfig.clientCertChainPath = ExampleMtlsConfig_GetClientCertChainPath();
        tlsStreamConfig.clientKeyPath       = ExampleMtlsConfig_GetClientKeyPath();
    }
    else
    {
        tlsStreamConfig.caBundlePath = ExampleTlsConfig_GetCaBundlePath();
        tlsStreamConfig.serverName   = ExampleTlsConfig_GetServerName();
    }
    tlsStream = SolidSyslogTlsStream_Create(&tlsStreamStorage, &tlsStreamConfig);

    static struct SolidSyslogStreamSenderConfig tlsSenderConfig = {0};
    tlsSenderConfig.resolver                                    = resolver;
    tlsSenderConfig.stream                                      = tlsStream;
    tlsSenderConfig.endpoint                                    = mtlsModeActive ? ExampleMtlsConfig_GetEndpoint : ExampleTlsConfig_GetEndpoint;
    tlsSenderConfig.endpointVersion                             = mtlsModeActive ? ExampleMtlsConfig_GetEndpointVersion : ExampleTlsConfig_GetEndpointVersion;
    tlsSender                                                   = SolidSyslogStreamSender_Create(&tlsSenderStorage, &tlsSenderConfig);
    tlsSlot                                                     = tlsSender;
#else
    (void) mtlsModeActive;
    tlsSlot = udpSender; /* fallback when TLS not built — keeps switching selector total */
#endif

    static struct SolidSyslogSender* inners[EXAMPLE_SWITCH_COUNT];
    inners[EXAMPLE_SWITCH_UDP] = udpSender;
    inners[EXAMPLE_SWITCH_TCP] = plainTcpSender;
    inners[EXAMPLE_SWITCH_TLS] = tlsSlot;

    static struct SolidSyslogSwitchingSenderConfig switchConfig = {0};
    switchConfig.senders                                        = inners;
    switchConfig.senderCount                                    = EXAMPLE_SWITCH_COUNT;
    switchConfig.selector                                       = ExampleSwitchConfig_Selector;

    ExampleSwitchConfig_SetByName(options->transport);
    return SolidSyslogSwitchingSender_Create(&switchConfig);
}

static enum SolidSyslogDiscardPolicy MapDiscardPolicy(const char* policy)
{
    if (strcmp(policy, "newest") == 0)
    {
        return SOLIDSYSLOG_DISCARD_NEWEST;
    }
    if (strcmp(policy, "halt") == 0)
    {
        return SOLIDSYSLOG_HALT;
    }
    return SOLIDSYSLOG_DISCARD_OLDEST;
}

static volatile bool haltExit;

static void OnStoreFull(void)
{
    if (haltExit)
    {
        _exit(2);
    }
}

static struct SolidSyslogStore* CreateStore(const struct ExampleOptions* options)
{
    bool useFile = (strcmp(options->store, "file") == 0);

    if (useFile)
    {
        static SolidSyslogPosixFileStorage readStorage;
        static SolidSyslogPosixFileStorage writeStorage;
        storeReadFile  = SolidSyslogPosixFile_Create(&readStorage);
        storeWriteFile = SolidSyslogPosixFile_Create(&writeStorage);

        static struct SolidSyslogFileStoreConfig storeConfig = {0};
        storeConfig.readFile                                 = storeReadFile;
        storeConfig.writeFile                                = storeWriteFile;
        storeConfig.pathPrefix                               = STORE_PATH_PREFIX;
        storeConfig.maxFileSize                              = options->maxFileSize;
        storeConfig.maxFiles                                 = options->maxFiles;
        storeConfig.discardPolicy                            = MapDiscardPolicy(options->discardPolicy);
        storeConfig.securityPolicy                           = SolidSyslogCrc16Policy_Create();
        storeConfig.onStoreFull                              = OnStoreFull;
        return SolidSyslogFileStore_Create(&storeConfig);
    }

    return SolidSyslogNullStore_Create();
}

static void DestroySender(void)
{
    SolidSyslogSwitchingSender_Destroy();
#ifdef SOLIDSYSLOG_HAVE_OPENSSL
    SolidSyslogStreamSender_Destroy(tlsSender);
    SolidSyslogTlsStream_Destroy(tlsStream);
    SolidSyslogPosixTcpStream_Destroy(tlsUnderlyingStream);
#endif
    SolidSyslogStreamSender_Destroy(plainTcpSender);
    SolidSyslogPosixTcpStream_Destroy(plainTcpStream);
    SolidSyslogUdpSender_Destroy();
    SolidSyslogPosixDatagram_Destroy();
    SolidSyslogGetAddrInfoResolver_Destroy();
}

static void DestroyStore(const struct ExampleOptions* options)
{
    bool useFile = (strcmp(options->store, "file") == 0);

    if (useFile)
    {
        SolidSyslogFileStore_Destroy();
        SolidSyslogCrc16Policy_Destroy();
        SolidSyslogPosixFile_Destroy(storeWriteFile);
        SolidSyslogPosixFile_Destroy(storeReadFile);
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

    struct SolidSyslogBuffer*         buffer  = SolidSyslogPosixMessageQueueBuffer_Create(SOLIDSYSLOG_MAX_MESSAGE_SIZE, 10);
    struct SolidSyslogAtomicCounter*  counter = SolidSyslogAtomicCounter_Create();
    struct SolidSyslogStructuredData* metaSd  = SolidSyslogMetaSd_Create(counter);

    struct SolidSyslogStructuredData* timeQuality = SolidSyslogTimeQualitySd_Create(GetTimeQuality);
    struct SolidSyslogStructuredData* originSd    = SolidSyslogOriginSd_Create("SolidSyslogExample", "0.7.0");

    struct SolidSyslogStructuredData* sdList[3] = {metaSd, timeQuality, originSd};
    size_t                            sdCount   = options.noSd ? 1 : 3;

    struct SolidSyslogConfig config = {
        .buffer       = buffer,
        .sender       = sender,
        .clock        = SolidSyslogPosixClock_GetTimestamp,
        .getHostname  = SolidSyslogPosixHostname_Get,
        .getAppName   = ExampleAppName_Get,
        .getProcessId = SolidSyslogPosixProcessId_Get,
        .store        = store,
        .sd           = sdList,
        .sdCount      = sdCount,
    };
    SolidSyslog_Create(&config);

    shutdown_flag = false;
    haltExit      = options.haltExit;

    pthread_t serviceThread = 0;
    pthread_create(&serviceThread, NULL, ServiceThreadEntry, (void*) &shutdown_flag);

    struct SolidSyslogMessage message = {
        .facility  = options.facility,
        .severity  = options.severity,
        .messageId = options.messageId,
        .msg       = options.msg,
    };

    ExampleInteractive_Run(&message, stdin, ExampleSwitchConfig_SetByName);

    shutdown_flag = true;
    pthread_join(serviceThread, NULL);

    SolidSyslog_Destroy();
    SolidSyslogOriginSd_Destroy();
    SolidSyslogTimeQualitySd_Destroy();
    SolidSyslogMetaSd_Destroy();
    SolidSyslogAtomicCounter_Destroy();
    DestroyStore(&options);
    SolidSyslogPosixMessageQueueBuffer_Destroy();
    DestroySender();

    return 0;
}
