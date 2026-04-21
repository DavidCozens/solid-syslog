#include "OpenSslFake.h"

#include <openssl/ssl.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------------------------------------------------------------
 * Captured state — one section per OpenSSL API call. Tests read these via
 * accessors below; production reaches libssl through the link-interposed
 * functions at the bottom of the file.
 * ------------------------------------------------------------------------- */

/* Sentinel storage for opaque OpenSSL types — our fake returns stable
 * pointers to these so tests can assert pointer-chain plumbing with
 * POINTERS_EQUAL. */
static char fakeCtxStorage;
static char fakeMethodStorage;
static char fakeSslStorage;
static char fakeBioMethStorage;
static char fakeBioStorage;

/* SSL_CTX_new */
static int               ctxNewCallCount;
static const SSL_METHOD* lastCtxNewMethodArg;

/* SSL_CTX_load_verify_locations */
static SSL_CTX*    lastLoadVerifyLocationsCtxArg;
static const char* lastCaBundlePath;

/* SSL_CTX_set_verify */
static SSL_CTX* lastSetVerifyCtxArg;
static int      lastVerifyMode;

/* SSL_CTX_ctrl (SET_MIN_PROTO_VERSION) */
static SSL_CTX* lastSslCtxCtrlCtxArg;
static long     lastMinProtoVersion;

/* SSL_new */
static int      sslNewCallCount;
static SSL_CTX* lastSslNewCtxArg;

/* BIO_meth_set_read / BIO_meth_set_write */
static BIO_METHOD* lastBioMethSetReadMethodArg;
static int (*lastBioReadCallback)(BIO*, char*, int);
static long (*lastBioCtrlCallback)(BIO*, int, long, void*);
static int (*lastBioCreateCallback)(BIO*);
static int         lastSetInitArg;
static BIO_METHOD* lastBioMethSetWriteMethodArg;
static int (*lastBioWriteCallback)(BIO*, const char*, int);

/* BIO_new */
static int               bioNewCallCount;
static const BIO_METHOD* lastBioNewMethodArg;

/* BIO_set_data / BIO_get_data */
static BIO*  lastSetDataBioArg;
static void* lastSetDataArg;
static BIO*  lastGetDataBioArg;

/* SSL_set_bio */
static int  setBioCallCount;
static SSL* lastSetBioSslArg;
static BIO* lastSetBioReadBioArg;
static BIO* lastSetBioWriteBioArg;

/* SSL_ctrl (SET_TLSEXT_HOSTNAME) */
static SSL*        lastSslCtrlSslArg;
static const char* lastSniHostname;

/* SSL_set1_host */
static SSL*        lastSet1HostSslArg;
static const char* lastSet1Host;

/* SSL_connect */
static int  connectCallCount;
static SSL* lastConnectSslArg;
static bool connectFails;

/* SSL_write */
static int         writeCallCount;
static SSL*        lastWriteSslArg;
static const void* lastWriteBuf;
static int         lastWriteSize;
static bool        writeFails;

/* SSL_shutdown */
static int  shutdownCallCount;
static SSL* lastShutdownSslArg;

/* SSL_free */
static int  freeCallCount;
static SSL* lastFreeSslArg;

/* SSL_CTX_free */
static int      ctxFreeCallCount;
static SSL_CTX* lastCtxFreeCtxArg;

/* -------------------------------------------------------------------------
 * Reset — zero every captured value.
 * ------------------------------------------------------------------------- */

void OpenSslFake_Reset(void)
{
    ctxNewCallCount               = 0;
    lastCtxNewMethodArg           = NULL;
    lastLoadVerifyLocationsCtxArg = NULL;
    lastCaBundlePath              = NULL;
    lastSetVerifyCtxArg           = NULL;
    lastVerifyMode                = 0;
    lastSslCtxCtrlCtxArg          = NULL;
    lastMinProtoVersion           = 0;
    sslNewCallCount               = 0;
    lastSslNewCtxArg              = NULL;
    lastBioMethSetReadMethodArg   = NULL;
    lastBioReadCallback           = NULL;
    lastBioCtrlCallback           = NULL;
    lastBioCreateCallback         = NULL;
    lastSetInitArg                = 0;
    lastBioMethSetWriteMethodArg  = NULL;
    lastBioWriteCallback          = NULL;
    bioNewCallCount               = 0;
    lastBioNewMethodArg           = NULL;
    lastSetDataBioArg             = NULL;
    lastSetDataArg                = NULL;
    lastGetDataBioArg             = NULL;
    setBioCallCount               = 0;
    lastSetBioSslArg              = NULL;
    lastSetBioReadBioArg          = NULL;
    lastSetBioWriteBioArg         = NULL;
    lastSslCtrlSslArg             = NULL;
    lastSniHostname               = NULL;
    lastSet1HostSslArg            = NULL;
    lastSet1Host                  = NULL;
    connectCallCount              = 0;
    lastConnectSslArg             = NULL;
    connectFails                  = false;
    writeCallCount                = 0;
    lastWriteSslArg               = NULL;
    lastWriteBuf                  = NULL;
    lastWriteSize                 = 0;
    writeFails                    = false;
    shutdownCallCount             = 0;
    lastShutdownSslArg            = NULL;
    freeCallCount                 = 0;
    lastFreeSslArg                = NULL;
    ctxFreeCallCount              = 0;
    lastCtxFreeCtxArg             = NULL;
}

/* -------------------------------------------------------------------------
 * Accessors — grouped by the OpenSSL function they describe.
 * ------------------------------------------------------------------------- */

int                OpenSslFake_CtxNewCallCount(void) { return ctxNewCallCount; }
const SSL_METHOD*  OpenSslFake_LastCtxNewMethodArg(void) { return lastCtxNewMethodArg; }
SSL_CTX*           OpenSslFake_LastCtxReturned(void) { return (SSL_CTX*) &fakeCtxStorage; }

SSL_CTX*           OpenSslFake_LastLoadVerifyLocationsCtxArg(void) { return lastLoadVerifyLocationsCtxArg; }
const char*        OpenSslFake_LastCaBundlePath(void) { return lastCaBundlePath; }

SSL_CTX*           OpenSslFake_LastSetVerifyCtxArg(void) { return lastSetVerifyCtxArg; }
int                OpenSslFake_LastVerifyMode(void) { return lastVerifyMode; }

SSL_CTX*           OpenSslFake_LastSslCtxCtrlCtxArg(void) { return lastSslCtxCtrlCtxArg; }
long               OpenSslFake_LastMinProtoVersion(void) { return lastMinProtoVersion; }

int                OpenSslFake_SslNewCallCount(void) { return sslNewCallCount; }
SSL_CTX*           OpenSslFake_LastSslNewCtxArg(void) { return lastSslNewCtxArg; }
SSL*               OpenSslFake_LastSslReturned(void) { return (SSL*) &fakeSslStorage; }

BIO_METHOD*        OpenSslFake_LastBioMethReturned(void) { return (BIO_METHOD*) &fakeBioMethStorage; }

BIO_METHOD*        OpenSslFake_LastBioMethSetReadMethodArg(void) { return lastBioMethSetReadMethodArg; }
int (*OpenSslFake_LastBioReadCallback(void))(BIO*, char*, int) { return lastBioReadCallback; }

long (*OpenSslFake_LastBioCtrlCallback(void))(BIO*, int, long, void*) { return lastBioCtrlCallback; }

int (*OpenSslFake_LastBioCreateCallback(void))(BIO*) { return lastBioCreateCallback; }

int OpenSslFake_LastSetInitArg(void) { return lastSetInitArg; }

BIO_METHOD*        OpenSslFake_LastBioMethSetWriteMethodArg(void) { return lastBioMethSetWriteMethodArg; }
int (*OpenSslFake_LastBioWriteCallback(void))(BIO*, const char*, int) { return lastBioWriteCallback; }

int                OpenSslFake_BioNewCallCount(void) { return bioNewCallCount; }
const BIO_METHOD*  OpenSslFake_LastBioNewMethodArg(void) { return lastBioNewMethodArg; }
BIO*               OpenSslFake_LastBioReturned(void) { return (BIO*) &fakeBioStorage; }

BIO*               OpenSslFake_LastSetDataBioArg(void) { return lastSetDataBioArg; }
void*              OpenSslFake_LastSetDataArg(void) { return lastSetDataArg; }
BIO*               OpenSslFake_LastGetDataBioArg(void) { return lastGetDataBioArg; }

int                OpenSslFake_SetBioCallCount(void) { return setBioCallCount; }
SSL*               OpenSslFake_LastSetBioSslArg(void) { return lastSetBioSslArg; }
BIO*               OpenSslFake_LastSetBioReadBioArg(void) { return lastSetBioReadBioArg; }
BIO*               OpenSslFake_LastSetBioWriteBioArg(void) { return lastSetBioWriteBioArg; }

SSL*               OpenSslFake_LastSslCtrlSslArg(void) { return lastSslCtrlSslArg; }
const char*        OpenSslFake_LastSniHostname(void) { return lastSniHostname; }

SSL*               OpenSslFake_LastSet1HostSslArg(void) { return lastSet1HostSslArg; }
const char*        OpenSslFake_LastSet1Host(void) { return lastSet1Host; }

int                OpenSslFake_ConnectCallCount(void) { return connectCallCount; }
SSL*               OpenSslFake_LastConnectSslArg(void) { return lastConnectSslArg; }

int                OpenSslFake_WriteCallCount(void) { return writeCallCount; }
SSL*               OpenSslFake_LastWriteSslArg(void) { return lastWriteSslArg; }
const void*        OpenSslFake_LastWriteBuf(void) { return lastWriteBuf; }
int                OpenSslFake_LastWriteSize(void) { return lastWriteSize; }

int                OpenSslFake_ShutdownCallCount(void) { return shutdownCallCount; }
SSL*               OpenSslFake_LastShutdownSslArg(void) { return lastShutdownSslArg; }

int                OpenSslFake_FreeCallCount(void) { return freeCallCount; }
SSL*               OpenSslFake_LastFreeSslArg(void) { return lastFreeSslArg; }

int                OpenSslFake_CtxFreeCallCount(void) { return ctxFreeCallCount; }
SSL_CTX*           OpenSslFake_LastCtxFreeCtxArg(void) { return lastCtxFreeCtxArg; }

/* -------------------------------------------------------------------------
 * Link-time substitution for OpenSSL — replaces libssl symbols in the test
 * binary. Production links real libssl; tests never link -lssl.
 * Each function records its args for test assertion. Return values are
 * plausible-success stubs; where behaviour needs switching for failure-path
 * tests, add a setter (e.g. OpenSslFake_SetConnectFails) in a later cycle.
 * ------------------------------------------------------------------------- */

const SSL_METHOD* TLS_client_method(void)
{
    return (const SSL_METHOD*) &fakeMethodStorage;
}

SSL_CTX* SSL_CTX_new(const SSL_METHOD* method)
{
    ctxNewCallCount++;
    lastCtxNewMethodArg = method;
    return (SSL_CTX*) &fakeCtxStorage;
}

int SSL_CTX_load_verify_locations(SSL_CTX* ctx, const char* CAfile, const char* CApath)
{
    (void) CApath;
    lastLoadVerifyLocationsCtxArg = ctx;
    lastCaBundlePath              = CAfile;
    return 1;
}

void SSL_CTX_set_verify(SSL_CTX* ctx, int mode, SSL_verify_cb verify_callback)
{
    (void) verify_callback;
    lastSetVerifyCtxArg = ctx;
    lastVerifyMode      = mode;
}

/* SSL_CTX_set_min_proto_version is a macro forwarding to SSL_CTX_ctrl; fake
 * intercepts the ctrl call for the min-proto command only. */
long SSL_CTX_ctrl(SSL_CTX* ctx, int cmd, long larg, void* parg)
{
    (void) parg;
    lastSslCtxCtrlCtxArg = ctx;
    if (cmd == SSL_CTRL_SET_MIN_PROTO_VERSION)
    {
        lastMinProtoVersion = larg;
    }
    return 1;
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

int BIO_meth_set_read(BIO_METHOD* method, int (*read)(BIO*, char*, int))
{
    lastBioMethSetReadMethodArg = method;
    lastBioReadCallback         = read;
    return 1;
}

int BIO_meth_set_write(BIO_METHOD* method, int (*write)(BIO*, const char*, int))
{
    lastBioMethSetWriteMethodArg = method;
    lastBioWriteCallback         = write;
    return 1;
}

int BIO_meth_set_ctrl(BIO_METHOD* method, long (*ctrl)(BIO*, int, long, void*))
{
    (void) method;
    lastBioCtrlCallback = ctrl;
    return 1;
}

int BIO_meth_set_create(BIO_METHOD* method, int (*create)(BIO*))
{
    (void) method;
    lastBioCreateCallback = create;
    return 1;
}

void BIO_set_init(BIO* bio, int init)
{
    (void) bio;
    lastSetInitArg = init;
}

BIO* BIO_new(const BIO_METHOD* method)
{
    bioNewCallCount++;
    lastBioNewMethodArg = method;
    return (BIO*) &fakeBioStorage;
}

void BIO_set_data(BIO* bio, void* data)
{
    lastSetDataBioArg = bio;
    lastSetDataArg    = data;
}

void* BIO_get_data(BIO* bio)
{
    lastGetDataBioArg = bio;
    return lastSetDataArg;
}

void SSL_set_bio(SSL* ssl, BIO* rbio, BIO* wbio)
{
    setBioCallCount++;
    lastSetBioSslArg      = ssl;
    lastSetBioReadBioArg  = rbio;
    lastSetBioWriteBioArg = wbio;
}

/* SSL_set_tlsext_host_name is a macro forwarding to SSL_ctrl; fake intercepts
 * the SET_TLSEXT_HOSTNAME command and captures the hostname pointer. */
long SSL_ctrl(SSL* ssl, int cmd, long larg, void* parg)
{
    (void) larg;
    lastSslCtrlSslArg = ssl;
    if (cmd == SSL_CTRL_SET_TLSEXT_HOSTNAME)
    {
        lastSniHostname = (const char*) parg;
    }
    return 1;
}

int SSL_set1_host(SSL* ssl, const char* hostname)
{
    lastSet1HostSslArg = ssl;
    lastSet1Host       = hostname;
    return 1;
}

int SSL_connect(SSL* ssl)
{
    connectCallCount++;
    lastConnectSslArg = ssl;
    return connectFails ? -1 : 1;
}

void OpenSslFake_SetConnectFails(bool fails)
{
    connectFails = fails;
}

int SSL_write(SSL* ssl, const void* buf, int num)
{
    writeCallCount++;
    lastWriteSslArg = ssl;
    lastWriteBuf    = buf;
    lastWriteSize   = num;
    return writeFails ? -1 : num;
}

void OpenSslFake_SetWriteFails(bool fails)
{
    writeFails = fails;
}

int SSL_shutdown(SSL* ssl)
{
    shutdownCallCount++;
    lastShutdownSslArg = ssl;
    return 1;
}

void SSL_free(SSL* ssl)
{
    freeCallCount++;
    lastFreeSslArg = ssl;
}

void SSL_CTX_free(SSL_CTX* ctx)
{
    ctxFreeCallCount++;
    lastCtxFreeCtxArg = ctx;
}
