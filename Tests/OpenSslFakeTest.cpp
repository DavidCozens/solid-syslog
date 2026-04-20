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
