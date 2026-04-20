#include "CppUTest/TestHarness.h"
#include "OpenSslFake.h"
#include "SolidSyslogAddress.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogTlsStream.h"
#include "StreamFake.h"
#include <openssl/ssl.h>

// clang-format off
TEST_GROUP(SolidSyslogTlsStream)
{
    struct SolidSyslogStream*         transport   = nullptr;
    struct SolidSyslogTlsStreamConfig config      = {};
    struct SolidSyslogStream*         stream      = nullptr;
    SolidSyslogAddressStorage         addrStorage = {0};
    struct SolidSyslogAddress*        addr        = nullptr;

    void setup() override
    {
        OpenSslFake_Reset();
        transport        = StreamFake_Create();
        config.transport = transport;
        stream           = SolidSyslogTlsStream_Create(&config);
        addr             = SolidSyslogAddress_FromStorage(&addrStorage);
    }

    void teardown() override
    {
        SolidSyslogTlsStream_Destroy();
        StreamFake_Destroy(transport);
    }
};
// clang-format on

TEST(SolidSyslogTlsStream, CreateSucceeds)
{
    CHECK_TRUE(stream != nullptr);
}

TEST(SolidSyslogTlsStream, OpenOpensTransport)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, StreamFake_OpenCallCount(transport));
}

TEST(SolidSyslogTlsStream, OpenPassesAddressToTransport)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(addr, StreamFake_LastOpenAddr(transport));
}

TEST(SolidSyslogTlsStream, OpenCreatesSslContext)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, OpenSslFake_CtxNewCallCount());
}

TEST(SolidSyslogTlsStream, OpenLoadsCaBundleFromConfig)
{
    SolidSyslogTlsStream_Destroy();
    config.caBundlePath = "/some/path/ca.pem";
    stream              = SolidSyslogTlsStream_Create(&config);
    SolidSyslogStream_Open(stream, addr);
    STRCMP_EQUAL("/some/path/ca.pem", OpenSslFake_LastCaBundlePath());
}

TEST(SolidSyslogTlsStream, OpenRequiresPeerVerification)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(SSL_VERIFY_PEER, OpenSslFake_LastVerifyMode());
}

TEST(SolidSyslogTlsStream, OpenSetsTls12Floor)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(TLS1_2_VERSION, OpenSslFake_LastMinProtoVersion());
}

TEST(SolidSyslogTlsStream, OpenCreatesSslSession)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, OpenSslFake_SslNewCallCount());
}

TEST(SolidSyslogTlsStream, OpenPassesCtxFromCtxNewToSslNew)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastCtxReturned(), OpenSslFake_LastSslNewCtxArg());
}

TEST(SolidSyslogTlsStream, OpenCreatesBio)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, OpenSslFake_BioNewCallCount());
}

TEST(SolidSyslogTlsStream, OpenSetsBioOnSsl)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, OpenSslFake_SetBioCallCount());
}

TEST(SolidSyslogTlsStream, OpenPassesSslFromNewToSetBio)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastSetBioSslArg());
}

TEST(SolidSyslogTlsStream, OpenPassesBioFromNewToSetBio)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastBioReturned(), OpenSslFake_LastSetBioReadBioArg());
}

TEST(SolidSyslogTlsStream, OpenPerformsHandshake)
{
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(1, OpenSslFake_ConnectCallCount());
}

TEST(SolidSyslogTlsStream, OpenPassesSslToConnect)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastConnectSslArg());
}

TEST(SolidSyslogTlsStream, OpenSetsSniHostnameFromConfig)
{
    SolidSyslogTlsStream_Destroy();
    config.serverName = "logs.example";
    stream            = SolidSyslogTlsStream_Create(&config);
    SolidSyslogStream_Open(stream, addr);
    STRCMP_EQUAL("logs.example", OpenSslFake_LastSniHostname());
}

TEST(SolidSyslogTlsStream, OpenSetsExpectedCertHostname)
{
    SolidSyslogTlsStream_Destroy();
    config.serverName = "logs.example";
    stream            = SolidSyslogTlsStream_Create(&config);
    SolidSyslogStream_Open(stream, addr);
    STRCMP_EQUAL("logs.example", OpenSslFake_LastSet1Host());
}

TEST(SolidSyslogTlsStream, OpenSkipsHostnameSetupWhenServerNameIsNull)
{
    /* Default config.serverName is NULL */
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(NULL, OpenSslFake_LastSet1Host());
}

TEST(SolidSyslogTlsStream, OpenAttachesTransportAsBioData)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(transport, OpenSslFake_LastSetDataArg());
}

TEST(SolidSyslogTlsStream, BioReadCallbackDelegatesToTransportRead)
{
    SolidSyslogStream_Open(stream, addr);
    int (*readFn)(BIO*, char*, int) = OpenSslFake_LastBioReadCallback();
    CHECK_TRUE(readFn != nullptr);
    char buf[16];
    readFn(OpenSslFake_LastBioReturned(), buf, sizeof(buf));
    LONGS_EQUAL(1, StreamFake_ReadCallCount(transport));
}

TEST(SolidSyslogTlsStream, BioWriteCallbackDelegatesToTransportSend)
{
    SolidSyslogStream_Open(stream, addr);
    int (*writeFn)(BIO*, const char*, int) = OpenSslFake_LastBioWriteCallback();
    CHECK_TRUE(writeFn != nullptr);
    const char msg[] = "hi";
    writeFn(OpenSslFake_LastBioReturned(), msg, (int) sizeof(msg));
    LONGS_EQUAL(1, StreamFake_SendCallCount(transport));
}

TEST(SolidSyslogTlsStream, SendWritesToSsl)
{
    SolidSyslogStream_Open(stream, addr);
    const char msg[] = "hello";
    SolidSyslogStream_Send(stream, msg, sizeof(msg));
    LONGS_EQUAL(1, OpenSslFake_WriteCallCount());
}

TEST(SolidSyslogTlsStream, SendPassesBufferToSslWrite)
{
    SolidSyslogStream_Open(stream, addr);
    const char msg[] = "hello";
    SolidSyslogStream_Send(stream, msg, sizeof(msg));
    POINTERS_EQUAL(msg, OpenSslFake_LastWriteBuf());
}

TEST(SolidSyslogTlsStream, SendPassesSizeToSslWrite)
{
    SolidSyslogStream_Open(stream, addr);
    const char msg[] = "hello";
    SolidSyslogStream_Send(stream, msg, sizeof(msg));
    LONGS_EQUAL(sizeof(msg), OpenSslFake_LastWriteSize());
}

TEST(SolidSyslogTlsStream, SendPassesSslFromNewToWrite)
{
    SolidSyslogStream_Open(stream, addr);
    const char msg[] = "hello";
    SolidSyslogStream_Send(stream, msg, sizeof(msg));
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastWriteSslArg());
}

TEST(SolidSyslogTlsStream, CloseShutsDownSsl)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(1, OpenSslFake_ShutdownCallCount());
}

TEST(SolidSyslogTlsStream, CloseFreesSsl)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(1, OpenSslFake_FreeCallCount());
}

TEST(SolidSyslogTlsStream, CloseClosesTransport)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    LONGS_EQUAL(1, StreamFake_CloseCallCount(transport));
}

TEST(SolidSyslogTlsStream, DestroyFreesSslContext)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogTlsStream_Destroy();
    LONGS_EQUAL(1, OpenSslFake_CtxFreeCallCount());
    /* teardown re-Destroys safely */
}

/* -------------------------------------------------------------------------
 * Pointer-chain assertions: each OpenSSL call must receive the handle
 * returned by the preceding call, not some stale or NULL pointer.
 * ------------------------------------------------------------------------- */

TEST(SolidSyslogTlsStream, OpenPassesClientMethodToCtxNew)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(TLS_client_method(), OpenSslFake_LastCtxNewMethodArg());
}

TEST(SolidSyslogTlsStream, OpenPassesCtxFromNewToLoadVerifyLocations)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastCtxReturned(), OpenSslFake_LastLoadVerifyLocationsCtxArg());
}

TEST(SolidSyslogTlsStream, OpenPassesCtxFromNewToSetVerify)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastCtxReturned(), OpenSslFake_LastSetVerifyCtxArg());
}

TEST(SolidSyslogTlsStream, OpenPassesCtxFromNewToSetMinProtoVersion)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastCtxReturned(), OpenSslFake_LastSslCtxCtrlCtxArg());
}

TEST(SolidSyslogTlsStream, OpenPassesBioMethodFromNewToSetRead)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastBioMethReturned(), OpenSslFake_LastBioMethSetReadMethodArg());
}

TEST(SolidSyslogTlsStream, OpenPassesBioMethodFromNewToSetWrite)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastBioMethReturned(), OpenSslFake_LastBioMethSetWriteMethodArg());
}

TEST(SolidSyslogTlsStream, OpenPassesBioMethodFromNewToBioNew)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastBioMethReturned(), OpenSslFake_LastBioNewMethodArg());
}

TEST(SolidSyslogTlsStream, OpenPassesBioFromNewToSetData)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastBioReturned(), OpenSslFake_LastSetDataBioArg());
}

TEST(SolidSyslogTlsStream, OpenPassesSameBioForReadAndWrite)
{
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastSetBioReadBioArg(), OpenSslFake_LastSetBioWriteBioArg());
}

TEST(SolidSyslogTlsStream, OpenPassesSslToSniCtrl)
{
    SolidSyslogTlsStream_Destroy();
    config.serverName = "logs.example";
    stream            = SolidSyslogTlsStream_Create(&config);
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastSslCtrlSslArg());
}

TEST(SolidSyslogTlsStream, OpenPassesSslFromNewToSet1Host)
{
    SolidSyslogTlsStream_Destroy();
    config.serverName = "logs.example";
    stream            = SolidSyslogTlsStream_Create(&config);
    SolidSyslogStream_Open(stream, addr);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastSet1HostSslArg());
}

TEST(SolidSyslogTlsStream, BioReadCallbackLooksUpDataOnTheCorrectBio)
{
    SolidSyslogStream_Open(stream, addr);
    int (*readFn)(BIO*, char*, int) = OpenSslFake_LastBioReadCallback();
    char buf[16];
    readFn(OpenSslFake_LastBioReturned(), buf, sizeof(buf));
    POINTERS_EQUAL(OpenSslFake_LastBioReturned(), OpenSslFake_LastGetDataBioArg());
}

TEST(SolidSyslogTlsStream, ClosePassesSslFromNewToShutdown)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastShutdownSslArg());
}

TEST(SolidSyslogTlsStream, ClosePassesSslFromNewToFree)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogStream_Close(stream);
    POINTERS_EQUAL(OpenSslFake_LastSslReturned(), OpenSslFake_LastFreeSslArg());
}

TEST(SolidSyslogTlsStream, DestroyPassesCtxFromNewToCtxFree)
{
    SolidSyslogStream_Open(stream, addr);
    SolidSyslogTlsStream_Destroy();
    POINTERS_EQUAL(OpenSslFake_LastCtxReturned(), OpenSslFake_LastCtxFreeCtxArg());
}

/* -------------------------------------------------------------------------
 * Error paths.
 * ------------------------------------------------------------------------- */

TEST(SolidSyslogTlsStream, OpenReturnsTrueOnHappyPath)
{
    CHECK_TRUE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogTlsStream, OpenReturnsFalseWhenHandshakeFails)
{
    OpenSslFake_SetConnectFails(true);
    CHECK_FALSE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogTlsStream, SendReturnsTrueOnHappyPath)
{
    SolidSyslogStream_Open(stream, addr);
    const char msg[] = "hi";
    CHECK_TRUE(SolidSyslogStream_Send(stream, msg, sizeof(msg)));
}

TEST(SolidSyslogTlsStream, SendReturnsFalseWhenWriteFails)
{
    SolidSyslogStream_Open(stream, addr);
    OpenSslFake_SetWriteFails(true);
    const char msg[] = "hi";
    CHECK_FALSE(SolidSyslogStream_Send(stream, msg, sizeof(msg)));
}

TEST(SolidSyslogTlsStream, OpenReturnsFalseWhenTransportOpenFails)
{
    StreamFake_SetOpenFails(transport, true);
    CHECK_FALSE(SolidSyslogStream_Open(stream, addr));
}

TEST(SolidSyslogTlsStream, OpenSkipsSslSetupWhenTransportOpenFails)
{
    StreamFake_SetOpenFails(transport, true);
    SolidSyslogStream_Open(stream, addr);
    LONGS_EQUAL(0, OpenSslFake_CtxNewCallCount());
}

TEST(SolidSyslogTlsStream, OpenWiresBioCtrlCallback)
{
    SolidSyslogStream_Open(stream, addr);
    CHECK_TRUE(OpenSslFake_LastBioCtrlCallback() != nullptr);
}

TEST(SolidSyslogTlsStream, OpenWiresBioCreateCallback)
{
    SolidSyslogStream_Open(stream, addr);
    CHECK_TRUE(OpenSslFake_LastBioCreateCallback() != nullptr);
}
