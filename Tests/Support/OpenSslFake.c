#include "OpenSslFake.h"

#include <openssl/ssl.h>
#include <stddef.h>

static int         ctxNewCallCount;
static const char* lastCaBundlePath;
static int         lastVerifyMode;
static long        lastMinProtoVersion;
static int         sslNewCallCount;
static SSL_CTX*    lastSslNewCtxArg;
static char        fakeCtxStorage;
static char        fakeMethodStorage;
static char        fakeSslStorage;

void OpenSslFake_Reset(void)
{
    ctxNewCallCount     = 0;
    lastCaBundlePath    = NULL;
    lastVerifyMode      = 0;
    lastMinProtoVersion = 0;
    sslNewCallCount     = 0;
    lastSslNewCtxArg    = NULL;
}

int OpenSslFake_CtxNewCallCount(void)
{
    return ctxNewCallCount;
}

const char* OpenSslFake_LastCaBundlePath(void)
{
    return lastCaBundlePath;
}

int OpenSslFake_LastVerifyMode(void)
{
    return lastVerifyMode;
}

long OpenSslFake_LastMinProtoVersion(void)
{
    return lastMinProtoVersion;
}

SSL_CTX* OpenSslFake_LastCtxReturned(void)
{
    return (SSL_CTX*) &fakeCtxStorage;
}

int OpenSslFake_SslNewCallCount(void)
{
    return sslNewCallCount;
}

SSL* OpenSslFake_LastSslReturned(void)
{
    return (SSL*) &fakeSslStorage;
}

SSL_CTX* OpenSslFake_LastSslNewCtxArg(void)
{
    return lastSslNewCtxArg;
}

/* Link-time substitution for OpenSSL — replaces libssl symbols in the test
 * binary. Production links real libssl; tests never link -lssl. */

const SSL_METHOD* TLS_client_method(void)
{
    return (const SSL_METHOD*) &fakeMethodStorage;
}

SSL_CTX* SSL_CTX_new(const SSL_METHOD* method)
{
    (void) method;
    ctxNewCallCount++;
    return (SSL_CTX*) &fakeCtxStorage;
}

SSL* SSL_new(SSL_CTX* ctx)
{
    sslNewCallCount++;
    lastSslNewCtxArg = ctx;
    return (SSL*) &fakeSslStorage;
}

int SSL_CTX_load_verify_locations(SSL_CTX* ctx, const char* CAfile, const char* CApath)
{
    (void) ctx;
    (void) CApath;
    lastCaBundlePath = CAfile;
    return 1;
}

void SSL_CTX_set_verify(SSL_CTX* ctx, int mode, SSL_verify_cb verify_callback)
{
    (void) ctx;
    (void) verify_callback;
    lastVerifyMode = mode;
}

/* SSL_CTX_set_min_proto_version is a macro forwarding to SSL_CTX_ctrl; fake by
 * intercepting the ctrl call for the min-proto command only. */
long SSL_CTX_ctrl(SSL_CTX* ctx, int cmd, long larg, void* parg)
{
    (void) ctx;
    (void) parg;
    if (cmd == SSL_CTRL_SET_MIN_PROTO_VERSION)
    {
        lastMinProtoVersion = larg;
    }
    return 1;
}
