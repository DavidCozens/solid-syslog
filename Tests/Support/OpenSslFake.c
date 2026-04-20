#include "OpenSslFake.h"

#include <openssl/ssl.h>
#include <stddef.h>

static int         ctxNewCallCount;
static const char* lastCaBundlePath;
static int         lastVerifyMode;
static long        lastMinProtoVersion;
static int         sslNewCallCount;
static SSL_CTX*    lastSslNewCtxArg;
static int         bioNewCallCount;
static int         setBioCallCount;
static SSL*        lastSetBioSslArg;
static BIO*        lastSetBioReadBioArg;
static int         connectCallCount;
static SSL*        lastConnectSslArg;
static const char* lastSniHostname;
static const char* lastSet1Host;
static void*       lastSetDataArg;
static int (*lastBioReadCallback)(BIO*, char*, int);
static int (*lastBioWriteCallback)(BIO*, const char*, int);
static char        fakeCtxStorage;
static char        fakeMethodStorage;
static char        fakeSslStorage;
static char        fakeBioMethStorage;
static char        fakeBioStorage;

void OpenSslFake_Reset(void)
{
    ctxNewCallCount     = 0;
    lastCaBundlePath    = NULL;
    lastVerifyMode      = 0;
    lastMinProtoVersion = 0;
    sslNewCallCount     = 0;
    lastSslNewCtxArg    = NULL;
    bioNewCallCount      = 0;
    setBioCallCount      = 0;
    lastSetBioSslArg     = NULL;
    lastSetBioReadBioArg = NULL;
    connectCallCount     = 0;
    lastConnectSslArg    = NULL;
    lastSniHostname      = NULL;
    lastSet1Host         = NULL;
    lastSetDataArg       = NULL;
    lastBioReadCallback  = NULL;
    lastBioWriteCallback = NULL;
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

int OpenSslFake_BioNewCallCount(void)
{
    return bioNewCallCount;
}

BIO* OpenSslFake_LastBioReturned(void)
{
    return (BIO*) &fakeBioStorage;
}

int OpenSslFake_SetBioCallCount(void)
{
    return setBioCallCount;
}

SSL* OpenSslFake_LastSetBioSslArg(void)
{
    return lastSetBioSslArg;
}

BIO* OpenSslFake_LastSetBioReadBioArg(void)
{
    return lastSetBioReadBioArg;
}

int OpenSslFake_ConnectCallCount(void)
{
    return connectCallCount;
}

SSL* OpenSslFake_LastConnectSslArg(void)
{
    return lastConnectSslArg;
}

const char* OpenSslFake_LastSniHostname(void)
{
    return lastSniHostname;
}

const char* OpenSslFake_LastSet1Host(void)
{
    return lastSet1Host;
}

void* OpenSslFake_LastSetDataArg(void)
{
    return lastSetDataArg;
}

int (*OpenSslFake_LastBioReadCallback(void))(BIO*, char*, int)
{
    return lastBioReadCallback;
}

int (*OpenSslFake_LastBioWriteCallback(void))(BIO*, const char*, int)
{
    return lastBioWriteCallback;
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

BIO_METHOD* BIO_meth_new(int type, const char* name)
{
    (void) type;
    (void) name;
    return (BIO_METHOD*) &fakeBioMethStorage;
}

BIO* BIO_new(const BIO_METHOD* method)
{
    (void) method;
    bioNewCallCount++;
    return (BIO*) &fakeBioStorage;
}

void SSL_set_bio(SSL* ssl, BIO* rbio, BIO* wbio)
{
    (void) wbio;
    setBioCallCount++;
    lastSetBioSslArg     = ssl;
    lastSetBioReadBioArg = rbio;
}

int SSL_connect(SSL* ssl)
{
    connectCallCount++;
    lastConnectSslArg = ssl;
    return 1;
}

/* SSL_set_tlsext_host_name is a macro forwarding to SSL_ctrl; fake intercepts
 * the SET_TLSEXT_HOSTNAME command and captures the hostname pointer. */
long SSL_ctrl(SSL* ssl, int cmd, long larg, void* parg)
{
    (void) ssl;
    (void) larg;
    if (cmd == SSL_CTRL_SET_TLSEXT_HOSTNAME)
    {
        lastSniHostname = (const char*) parg;
    }
    return 1;
}

int SSL_set1_host(SSL* ssl, const char* hostname)
{
    (void) ssl;
    lastSet1Host = hostname;
    return 1;
}

void BIO_set_data(BIO* bio, void* data)
{
    (void) bio;
    lastSetDataArg = data;
}

void* BIO_get_data(BIO* bio)
{
    (void) bio;
    return lastSetDataArg;
}

int BIO_meth_set_read(BIO_METHOD* method, int (*read)(BIO*, char*, int))
{
    (void) method;
    lastBioReadCallback = read;
    return 1;
}

int BIO_meth_set_write(BIO_METHOD* method, int (*write)(BIO*, const char*, int))
{
    (void) method;
    lastBioWriteCallback = write;
    return 1;
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
