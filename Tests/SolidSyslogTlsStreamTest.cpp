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
