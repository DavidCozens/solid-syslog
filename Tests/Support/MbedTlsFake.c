#include "MbedTlsFake.h"

#include <mbedtls/ssl.h>
#include <stdbool.h>
#include <stddef.h>

/* -------------------------------------------------------------------------
 * Captured state — one section per mbedTLS API call. Tests read these via
 * accessors below; production reaches libmbedtls through the link-interposed
 * functions at the bottom of the file.
 * ------------------------------------------------------------------------- */

/* mbedtls_ssl_config_init */
static int sslConfigInitCallCount;
static mbedtls_ssl_config* lastSslConfigInitArg;

/* mbedtls_ssl_config_defaults */
static int sslConfigDefaultsCallCount;
static mbedtls_ssl_config* lastSslConfigDefaultsConfigArg;
static int lastSslConfigDefaultsEndpoint;
static int lastSslConfigDefaultsTransport;
static int lastSslConfigDefaultsPreset;

/* mbedtls_ssl_init */
static int sslInitCallCount;
static mbedtls_ssl_context* lastSslInitArg;

/* mbedtls_ssl_setup */
static int sslSetupCallCount;
static mbedtls_ssl_context* lastSslSetupContextArg;
static const mbedtls_ssl_config* lastSslSetupConfigArg;

/* mbedtls_ssl_set_bio */
static int sslSetBioCallCount;
static mbedtls_ssl_context* lastSslSetBioContextArg;
static void* lastSslSetBioPBioArg;
static mbedtls_ssl_send_t* lastSslSetBioSendCallback;
static mbedtls_ssl_recv_t* lastSslSetBioRecvCallback;
static mbedtls_ssl_recv_timeout_t* lastSslSetBioRecvTimeoutCallback;

/* mbedtls_ssl_handshake */
static int sslHandshakeCallCount;
static mbedtls_ssl_context* lastSslHandshakeArg;
static int sslHandshakeReturn;

/* mbedtls_ssl_write */
static int sslWriteCallCount;
static mbedtls_ssl_context* lastSslWriteContextArg;
static const unsigned char* lastSslWriteBufArg;
static size_t lastSslWriteLenArg;
static int sslWriteReturn;
static bool sslWriteReturnSet;

/* mbedtls_ssl_read */
static int sslReadCallCount;
static mbedtls_ssl_context* lastSslReadContextArg;
static unsigned char* lastSslReadBufArg;
static size_t lastSslReadLenArg;
static int sslReadReturn;

/* mbedtls_ssl_close_notify */
static int sslCloseNotifyCallCount;
static mbedtls_ssl_context* lastSslCloseNotifyArg;

/* mbedtls_ssl_free */
static int sslFreeCallCount;
static mbedtls_ssl_context* lastSslFreeArg;

/* mbedtls_ssl_config_free */
static int sslConfigFreeCallCount;
static mbedtls_ssl_config* lastSslConfigFreeArg;

/* -------------------------------------------------------------------------
 * Test accessors.
 * ------------------------------------------------------------------------- */

void MbedTlsFake_Reset(void)
{
    sslConfigInitCallCount = 0;
    lastSslConfigInitArg = NULL;
    sslConfigDefaultsCallCount = 0;
    lastSslConfigDefaultsConfigArg = NULL;
    lastSslConfigDefaultsEndpoint = 0;
    lastSslConfigDefaultsTransport = 0;
    lastSslConfigDefaultsPreset = 0;
    sslInitCallCount = 0;
    lastSslInitArg = NULL;
    sslSetupCallCount = 0;
    lastSslSetupContextArg = NULL;
    lastSslSetupConfigArg = NULL;
    sslSetBioCallCount = 0;
    lastSslSetBioContextArg = NULL;
    lastSslSetBioPBioArg = NULL;
    lastSslSetBioSendCallback = NULL;
    lastSslSetBioRecvCallback = NULL;
    lastSslSetBioRecvTimeoutCallback = NULL;
    sslHandshakeCallCount = 0;
    lastSslHandshakeArg = NULL;
    sslHandshakeReturn = 0;
    sslWriteCallCount = 0;
    lastSslWriteContextArg = NULL;
    lastSslWriteBufArg = NULL;
    lastSslWriteLenArg = 0;
    sslWriteReturn = 0;
    sslWriteReturnSet = false;
    sslReadCallCount = 0;
    lastSslReadContextArg = NULL;
    lastSslReadBufArg = NULL;
    lastSslReadLenArg = 0;
    sslReadReturn = 0;
    sslCloseNotifyCallCount = 0;
    lastSslCloseNotifyArg = NULL;
    sslFreeCallCount = 0;
    lastSslFreeArg = NULL;
    sslConfigFreeCallCount = 0;
    lastSslConfigFreeArg = NULL;
}

int MbedTlsFake_SslConfigInitCallCount(void)
{
    return sslConfigInitCallCount;
}

mbedtls_ssl_config* MbedTlsFake_LastSslConfigInitArg(void)
{
    return lastSslConfigInitArg;
}

int MbedTlsFake_SslConfigDefaultsCallCount(void)
{
    return sslConfigDefaultsCallCount;
}

mbedtls_ssl_config* MbedTlsFake_LastSslConfigDefaultsConfigArg(void)
{
    return lastSslConfigDefaultsConfigArg;
}

int MbedTlsFake_LastSslConfigDefaultsEndpoint(void)
{
    return lastSslConfigDefaultsEndpoint;
}

int MbedTlsFake_LastSslConfigDefaultsTransport(void)
{
    return lastSslConfigDefaultsTransport;
}

int MbedTlsFake_LastSslConfigDefaultsPreset(void)
{
    return lastSslConfigDefaultsPreset;
}

int MbedTlsFake_SslInitCallCount(void)
{
    return sslInitCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslInitArg(void)
{
    return lastSslInitArg;
}

int MbedTlsFake_SslSetupCallCount(void)
{
    return sslSetupCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslSetupContextArg(void)
{
    return lastSslSetupContextArg;
}

const mbedtls_ssl_config* MbedTlsFake_LastSslSetupConfigArg(void)
{
    return lastSslSetupConfigArg;
}

int MbedTlsFake_SslSetBioCallCount(void)
{
    return sslSetBioCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslSetBioContextArg(void)
{
    return lastSslSetBioContextArg;
}

void* MbedTlsFake_LastSslSetBioPBioArg(void)
{
    return lastSslSetBioPBioArg;
}

mbedtls_ssl_send_t* MbedTlsFake_LastSslSetBioSendCallback(void)
{
    return lastSslSetBioSendCallback;
}

mbedtls_ssl_recv_t* MbedTlsFake_LastSslSetBioRecvCallback(void)
{
    return lastSslSetBioRecvCallback;
}

int MbedTlsFake_SslHandshakeCallCount(void)
{
    return sslHandshakeCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslHandshakeArg(void)
{
    return lastSslHandshakeArg;
}

void MbedTlsFake_SetSslHandshakeReturn(int value)
{
    sslHandshakeReturn = value;
}

int MbedTlsFake_SslWriteCallCount(void)
{
    return sslWriteCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslWriteContextArg(void)
{
    return lastSslWriteContextArg;
}

const unsigned char* MbedTlsFake_LastSslWriteBufArg(void)
{
    return lastSslWriteBufArg;
}

size_t MbedTlsFake_LastSslWriteLenArg(void)
{
    return lastSslWriteLenArg;
}

void MbedTlsFake_SetSslWriteReturn(int value)
{
    sslWriteReturn = value;
    sslWriteReturnSet = true;
}

int MbedTlsFake_SslReadCallCount(void)
{
    return sslReadCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslReadContextArg(void)
{
    return lastSslReadContextArg;
}

unsigned char* MbedTlsFake_LastSslReadBufArg(void)
{
    return lastSslReadBufArg;
}

size_t MbedTlsFake_LastSslReadLenArg(void)
{
    return lastSslReadLenArg;
}

void MbedTlsFake_SetSslReadReturn(int value)
{
    sslReadReturn = value;
}

int MbedTlsFake_SslCloseNotifyCallCount(void)
{
    return sslCloseNotifyCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslCloseNotifyArg(void)
{
    return lastSslCloseNotifyArg;
}

int MbedTlsFake_SslFreeCallCount(void)
{
    return sslFreeCallCount;
}

mbedtls_ssl_context* MbedTlsFake_LastSslFreeArg(void)
{
    return lastSslFreeArg;
}

int MbedTlsFake_SslConfigFreeCallCount(void)
{
    return sslConfigFreeCallCount;
}

mbedtls_ssl_config* MbedTlsFake_LastSslConfigFreeArg(void)
{
    return lastSslConfigFreeArg;
}

/* -------------------------------------------------------------------------
 * Link-interposed mbedTLS symbols. The test executable does not link
 * libmbedtls; the production code's calls to mbedtls_* resolve here.
 * ------------------------------------------------------------------------- */

void mbedtls_ssl_config_init(mbedtls_ssl_config* conf)
{
    sslConfigInitCallCount++;
    lastSslConfigInitArg = conf;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- signature fixed by mbedTLS API
int mbedtls_ssl_config_defaults(mbedtls_ssl_config* conf, int endpoint, int transport, int preset)
{
    sslConfigDefaultsCallCount++;
    lastSslConfigDefaultsConfigArg = conf;
    lastSslConfigDefaultsEndpoint = endpoint;
    lastSslConfigDefaultsTransport = transport;
    lastSslConfigDefaultsPreset = preset;
    return 0;
}

void mbedtls_ssl_init(mbedtls_ssl_context* ssl)
{
    sslInitCallCount++;
    lastSslInitArg = ssl;
}

int mbedtls_ssl_setup(mbedtls_ssl_context* ssl, const mbedtls_ssl_config* conf)
{
    sslSetupCallCount++;
    lastSslSetupContextArg = ssl;
    lastSslSetupConfigArg = conf;
    return 0;
}

void mbedtls_ssl_set_bio(
    mbedtls_ssl_context* ssl,
    void* p_bio,
    mbedtls_ssl_send_t* f_send,
    mbedtls_ssl_recv_t* f_recv,
    mbedtls_ssl_recv_timeout_t* f_recv_timeout
)
{
    sslSetBioCallCount++;
    lastSslSetBioContextArg = ssl;
    lastSslSetBioPBioArg = p_bio;
    lastSslSetBioSendCallback = f_send;
    lastSslSetBioRecvCallback = f_recv;
    lastSslSetBioRecvTimeoutCallback = f_recv_timeout;
}

int mbedtls_ssl_handshake(mbedtls_ssl_context* ssl)
{
    sslHandshakeCallCount++;
    lastSslHandshakeArg = ssl;
    return sslHandshakeReturn;
}

int mbedtls_ssl_write(mbedtls_ssl_context* ssl, const unsigned char* buf, size_t len)
{
    sslWriteCallCount++;
    lastSslWriteContextArg = ssl;
    lastSslWriteBufArg = buf;
    lastSslWriteLenArg = len;
    /* Default: write succeeds fully (returns len). Tests can override via SetSslWriteReturn. */
    return sslWriteReturnSet ? sslWriteReturn : (int) len;
}

int mbedtls_ssl_read(mbedtls_ssl_context* ssl, unsigned char* buf, size_t len)
{
    sslReadCallCount++;
    lastSslReadContextArg = ssl;
    lastSslReadBufArg = buf;
    lastSslReadLenArg = len;
    return sslReadReturn;
}

int mbedtls_ssl_close_notify(mbedtls_ssl_context* ssl)
{
    sslCloseNotifyCallCount++;
    lastSslCloseNotifyArg = ssl;
    return 0;
}

void mbedtls_ssl_free(mbedtls_ssl_context* ssl)
{
    sslFreeCallCount++;
    lastSslFreeArg = ssl;
}

void mbedtls_ssl_config_free(mbedtls_ssl_config* conf)
{
    sslConfigFreeCallCount++;
    lastSslConfigFreeArg = conf;
}
