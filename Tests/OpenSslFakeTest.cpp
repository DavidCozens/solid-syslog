#include "CppUTest/TestHarness.h"
#include "OpenSslFake.h"
#include <openssl/ssl.h>

TEST_GROUP(OpenSslFake)
{
    void setup() override { OpenSslFake_Reset(); }
};

TEST(OpenSslFake, CtxNewCountIsZeroAfterReset)
{
    LONGS_EQUAL(0, OpenSslFake_CtxNewCallCount());
}

TEST(OpenSslFake, CtxNewIncrementsCount)
{
    SSL_CTX_new(TLS_client_method());
    LONGS_EQUAL(1, OpenSslFake_CtxNewCallCount());
}

TEST(OpenSslFake, CtxNewReturnsNonNull)
{
    CHECK_TRUE(SSL_CTX_new(TLS_client_method()) != nullptr);
}

TEST(OpenSslFake, LoadVerifyLocationsCapturesCaPath)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_load_verify_locations(ctx, "/my/ca.pem", nullptr);
    STRCMP_EQUAL("/my/ca.pem", OpenSslFake_LastCaBundlePath());
}

TEST(OpenSslFake, SetVerifyCapturesMode)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);
    LONGS_EQUAL(SSL_VERIFY_PEER, OpenSslFake_LastVerifyMode());
}

TEST(OpenSslFake, SetMinProtoVersionCapturesVersion)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    LONGS_EQUAL(TLS1_2_VERSION, OpenSslFake_LastMinProtoVersion());
}

TEST(OpenSslFake, CtxNewReturnValueIsSurfaced)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    POINTERS_EQUAL(ctx, OpenSslFake_LastCtxReturned());
}

TEST(OpenSslFake, SslNewIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_new(ctx);
    LONGS_EQUAL(1, OpenSslFake_SslNewCallCount());
}

TEST(OpenSslFake, SslNewReturnsNonNull)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    CHECK_TRUE(SSL_new(ctx) != nullptr);
}

TEST(OpenSslFake, SslNewCapturesCtxArg)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_new(ctx);
    POINTERS_EQUAL(ctx, OpenSslFake_LastSslNewCtxArg());
}

TEST(OpenSslFake, BioNewIncrementsCount)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO_new(method);
    LONGS_EQUAL(1, OpenSslFake_BioNewCallCount());
}

TEST(OpenSslFake, BioNewReturnsNonNull)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    CHECK_TRUE(BIO_new(method) != nullptr);
}

TEST(OpenSslFake, BioNewReturnValueIsSurfaced)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    POINTERS_EQUAL(bio, OpenSslFake_LastBioReturned());
}

TEST(OpenSslFake, SetBioIncrementsCount)
{
    SSL_CTX*    ctx    = SSL_CTX_new(TLS_client_method());
    SSL*        ssl    = SSL_new(ctx);
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    SSL_set_bio(ssl, bio, bio);
    LONGS_EQUAL(1, OpenSslFake_SetBioCallCount());
}

TEST(OpenSslFake, SetBioCapturesSslArg)
{
    SSL_CTX*    ctx    = SSL_CTX_new(TLS_client_method());
    SSL*        ssl    = SSL_new(ctx);
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    SSL_set_bio(ssl, bio, bio);
    POINTERS_EQUAL(ssl, OpenSslFake_LastSetBioSslArg());
}

TEST(OpenSslFake, SetBioCapturesReadBioArg)
{
    SSL_CTX*    ctx    = SSL_CTX_new(TLS_client_method());
    SSL*        ssl    = SSL_new(ctx);
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    SSL_set_bio(ssl, bio, bio);
    POINTERS_EQUAL(bio, OpenSslFake_LastSetBioReadBioArg());
}

TEST(OpenSslFake, ConnectIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_connect(ssl);
    LONGS_EQUAL(1, OpenSslFake_ConnectCallCount());
}

TEST(OpenSslFake, ConnectCapturesSslArg)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_connect(ssl);
    POINTERS_EQUAL(ssl, OpenSslFake_LastConnectSslArg());
}

TEST(OpenSslFake, ConnectDefaultsToSuccess)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    LONGS_EQUAL(1, SSL_connect(ssl));
}

TEST(OpenSslFake, SetTlsExtHostNameCapturesHostname)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_set_tlsext_host_name(ssl, "host.example");
    STRCMP_EQUAL("host.example", OpenSslFake_LastSniHostname());
}

TEST(OpenSslFake, Set1HostCapturesHostname)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_set1_host(ssl, "host.example");
    STRCMP_EQUAL("host.example", OpenSslFake_LastSet1Host());
}

TEST(OpenSslFake, BioSetDataCapturesData)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    int         sentinel;
    BIO_set_data(bio, &sentinel);
    POINTERS_EQUAL(&sentinel, OpenSslFake_LastSetDataArg());
}

TEST(OpenSslFake, BioGetDataReturnsPreviouslySetData)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO*        bio    = BIO_new(method);
    int         sentinel;
    BIO_set_data(bio, &sentinel);
    POINTERS_EQUAL(&sentinel, BIO_get_data(bio));
}

static int DummyRead(BIO* bio, char* buf, int size)
{
    (void) bio;
    (void) buf;
    (void) size;
    return 0;
}

TEST(OpenSslFake, BioMethSetReadCapturesCallback)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO_meth_set_read(method, DummyRead);
    POINTERS_EQUAL((void*) DummyRead, (void*) OpenSslFake_LastBioReadCallback());
}

static int DummyWrite(BIO* bio, const char* buf, int size)
{
    (void) bio;
    (void) buf;
    (void) size;
    return 0;
}

TEST(OpenSslFake, BioMethSetWriteCapturesCallback)
{
    BIO_METHOD* method = BIO_meth_new(0, "fake");
    BIO_meth_set_write(method, DummyWrite);
    POINTERS_EQUAL((void*) DummyWrite, (void*) OpenSslFake_LastBioWriteCallback());
}

TEST(OpenSslFake, WriteIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_write(ssl, "x", 1);
    LONGS_EQUAL(1, OpenSslFake_WriteCallCount());
}

TEST(OpenSslFake, WriteCapturesSslArg)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_write(ssl, "x", 1);
    POINTERS_EQUAL(ssl, OpenSslFake_LastWriteSslArg());
}

TEST(OpenSslFake, WriteCapturesBuffer)
{
    SSL_CTX*    ctx = SSL_CTX_new(TLS_client_method());
    SSL*        ssl = SSL_new(ctx);
    const char* msg = "payload";
    SSL_write(ssl, msg, 7);
    POINTERS_EQUAL(msg, OpenSslFake_LastWriteBuf());
}

TEST(OpenSslFake, WriteCapturesSize)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_write(ssl, "payload", 7);
    LONGS_EQUAL(7, OpenSslFake_LastWriteSize());
}

TEST(OpenSslFake, WriteDefaultsToEchoingSize)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    LONGS_EQUAL(7, SSL_write(ssl, "payload", 7));
}

TEST(OpenSslFake, ShutdownIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_shutdown(ssl);
    LONGS_EQUAL(1, OpenSslFake_ShutdownCallCount());
}

TEST(OpenSslFake, FreeIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL*     ssl = SSL_new(ctx);
    SSL_free(ssl);
    LONGS_EQUAL(1, OpenSslFake_FreeCallCount());
}

TEST(OpenSslFake, CtxFreeIncrementsCount)
{
    SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_free(ctx);
    LONGS_EQUAL(1, OpenSslFake_CtxFreeCallCount());
}
