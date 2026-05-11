#include <stdbool.h>

#include "BddTargetMtlsConfig.h"
#include "BddTargetTlsConfig.h"
#include "BddTargetTlsSender.h"
#include "SolidSyslogStreamSender.h"
#include "SolidSyslogTlsStream.h"
#include "SolidSyslogWindowsSleep.h"
#include "SolidSyslogWinsockTcpStream.h"

struct SolidSyslogResolver;

static SolidSyslogWinsockTcpStreamStorage underlyingStreamStorage;
static struct SolidSyslogStream*          underlyingStream;
static SolidSyslogTlsStreamStorage        tlsStreamStorage;
static struct SolidSyslogStream*          tlsStream;
static SolidSyslogStreamSenderStorage     senderStorage;
static struct SolidSyslogSender*          sender;

struct SolidSyslogSender* BddTargetTlsSender_Create(struct SolidSyslogResolver* resolver, bool mtls)
{
    underlyingStream = SolidSyslogWinsockTcpStream_Create(&underlyingStreamStorage);

    static struct SolidSyslogTlsStreamConfig tlsStreamConfig;
    tlsStreamConfig           = (struct SolidSyslogTlsStreamConfig) {0};
    tlsStreamConfig.transport = underlyingStream;
    tlsStreamConfig.sleep     = SolidSyslogWindowsSleep;
    if (mtls)
    {
        tlsStreamConfig.caBundlePath        = BddTargetMtlsConfig_GetCaBundlePath();
        tlsStreamConfig.serverName          = BddTargetMtlsConfig_GetServerName();
        tlsStreamConfig.clientCertChainPath = BddTargetMtlsConfig_GetClientCertChainPath();
        tlsStreamConfig.clientKeyPath       = BddTargetMtlsConfig_GetClientKeyPath();
    }
    else
    {
        tlsStreamConfig.caBundlePath = BddTargetTlsConfig_GetCaBundlePath();
        tlsStreamConfig.serverName   = BddTargetTlsConfig_GetServerName();
    }
    tlsStream = SolidSyslogTlsStream_Create(&tlsStreamStorage, &tlsStreamConfig);

    static struct SolidSyslogStreamSenderConfig senderConfig;
    senderConfig                 = (struct SolidSyslogStreamSenderConfig) {0};
    senderConfig.resolver        = resolver;
    senderConfig.stream          = tlsStream;
    senderConfig.endpoint        = mtls ? BddTargetMtlsConfig_GetEndpoint : BddTargetTlsConfig_GetEndpoint;
    senderConfig.endpointVersion = mtls ? BddTargetMtlsConfig_GetEndpointVersion : BddTargetTlsConfig_GetEndpointVersion;
    sender                       = SolidSyslogStreamSender_Create(&senderStorage, &senderConfig);

    return sender;
}

void BddTargetTlsSender_Destroy(void)
{
    SolidSyslogStreamSender_Destroy(sender);
    SolidSyslogTlsStream_Destroy(tlsStream);
    SolidSyslogWinsockTcpStream_Destroy(underlyingStream);
}
