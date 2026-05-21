#include "SolidSyslogMbedTlsStream.h"

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ssl.h>
#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogMbedTlsStreamPrivate.h"
#include "SolidSyslogNullStream.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogStreamDefinition.h"

enum
{
    /* Bounded retry budget for the TLS handshake under non-blocking transport.
       Mirrors the OpenSSL TlsStream constants (5s comfortably covers WAN
       deployments without burning the service thread indefinitely on a
       wedged peer). */
    HANDSHAKE_TIMEOUT_MILLISECONDS = 5000,
    HANDSHAKE_POLL_INTERVAL_MILLISECONDS = 1
};

struct SolidSyslogAddress;

static inline struct SolidSyslogMbedTlsStream* MbedTlsStream_SelfFromBase(struct SolidSyslogStream* base);
static inline bool MbedTlsStream_Open(struct SolidSyslogStream* base, const struct SolidSyslogAddress* addr);
static inline bool MbedTlsStream_PerformHandshake(struct SolidSyslogMbedTlsStream* self);
static inline bool MbedTlsStream_IsRetryableHandshakeRc(int rc);
static inline bool MbedTlsStream_IsHandshakeBudgetExhausted(int totalSleptMs);
static inline bool MbedTlsStream_Send(struct SolidSyslogStream* base, const void* buffer, size_t size);
static inline SolidSyslogSsize MbedTlsStream_Read(struct SolidSyslogStream* base, void* buffer, size_t size);
static inline void MbedTlsStream_Close(struct SolidSyslogStream* base);
static int MbedTlsStream_BioSend(void* ctx, const unsigned char* buf, size_t len);
static int MbedTlsStream_BioRecv(void* ctx, unsigned char* buf, size_t len);

void MbedTlsStream_Initialise(struct SolidSyslogStream* base, const struct SolidSyslogMbedTlsStreamConfig* config)
{
    struct SolidSyslogMbedTlsStream* self = MbedTlsStream_SelfFromBase(base);
    /* Read / Close inherit NullStream defaults until slice-2 tests force
     * their real implementations. */
    self->Base = *SolidSyslogNullStream_Get();
    self->Base.Open = MbedTlsStream_Open;
    self->Base.Send = MbedTlsStream_Send;
    self->Base.Read = MbedTlsStream_Read;
    self->Base.Close = MbedTlsStream_Close;
    self->Config = *config;
    self->IsOpen = false;
}

void MbedTlsStream_Cleanup(struct SolidSyslogStream* base)
{
    /* Mirror the OpenSSL TlsStream pattern: an integrator who destroys a
     * still-Open stream must not leak the underlying TLS state. Close
     * guards on IsOpen so Cleanup is safe even when Open was never reached. */
    MbedTlsStream_Close(base);
    /* Overwrite the abstract base with the shared NullStream vtable so
     * use-after-destroy is a safe no-op rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullStream_Get();
}

static inline struct SolidSyslogMbedTlsStream* MbedTlsStream_SelfFromBase(struct SolidSyslogStream* base)
{
    return (struct SolidSyslogMbedTlsStream*) base;
}

static inline bool MbedTlsStream_Open(struct SolidSyslogStream* base, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogMbedTlsStream* self = MbedTlsStream_SelfFromBase(base);
    bool ok = SolidSyslogStream_Open(self->Config.Transport, addr);
    if (ok)
    {
        mbedtls_ssl_config_init(&self->SslConfig);
        /* Flip IsOpen now so any subsequent allocation failure in this
         * function still leaves the struct in a Close-safe state — the
         * partially-initialised config/context must reach mbedtls_*_free. */
        self->IsOpen = true;
        ok = mbedtls_ssl_config_defaults(
                 &self->SslConfig,
                 MBEDTLS_SSL_IS_CLIENT,
                 MBEDTLS_SSL_TRANSPORT_STREAM,
                 MBEDTLS_SSL_PRESET_DEFAULT
             ) == 0;
    }
    if (ok)
    {
        mbedtls_ssl_conf_authmode(&self->SslConfig, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&self->SslConfig, self->Config.CaChain, NULL);
        mbedtls_ssl_conf_rng(&self->SslConfig, mbedtls_ctr_drbg_random, self->Config.Rng);
        if ((self->Config.ClientCertChain != NULL) && (self->Config.ClientKey != NULL))
        {
            (void) mbedtls_ssl_conf_own_cert(&self->SslConfig, self->Config.ClientCertChain, self->Config.ClientKey);
        }
    }
    if (ok)
    {
        mbedtls_ssl_init(&self->SslContext);
        ok = mbedtls_ssl_setup(&self->SslContext, &self->SslConfig) == 0;
    }
    if (ok && (self->Config.ServerName != NULL))
    {
        ok = mbedtls_ssl_set_hostname(&self->SslContext, self->Config.ServerName) == 0;
    }
    if (ok)
    {
        mbedtls_ssl_set_bio(&self->SslContext, self, MbedTlsStream_BioSend, MbedTlsStream_BioRecv, NULL);
        ok = MbedTlsStream_PerformHandshake(self);
    }
    return ok;
}

/* Drive mbedtls_ssl_handshake to completion under non-blocking transport.
 * Each call may return WANT_READ/WANT_WRITE while waiting for the multi-RTT
 * handshake to progress; we sleep briefly between attempts (avoiding a busy
 * spin) until either the handshake completes, hits a hard error, or the
 * bounded budget expires. Same shape as OpenSSL's TlsStream_PerformHandshake. */
static inline bool MbedTlsStream_PerformHandshake(struct SolidSyslogMbedTlsStream* self)
{
    int totalSleptMs = 0;
    bool result = false;
    bool done = false;

    while (!done)
    {
        int rc = mbedtls_ssl_handshake(&self->SslContext);
        if (rc == 0)
        {
            result = true;
            done = true;
        }
        else if (!MbedTlsStream_IsRetryableHandshakeRc(rc) || MbedTlsStream_IsHandshakeBudgetExhausted(totalSleptMs))
        {
            done = true;
        }
        else
        {
            self->Config.Sleep(HANDSHAKE_POLL_INTERVAL_MILLISECONDS);
            totalSleptMs += HANDSHAKE_POLL_INTERVAL_MILLISECONDS;
        }
    }
    return result;
}

static inline bool MbedTlsStream_IsRetryableHandshakeRc(int rc)
{
    return (rc == MBEDTLS_ERR_SSL_WANT_READ) || (rc == MBEDTLS_ERR_SSL_WANT_WRITE);
}

static inline bool MbedTlsStream_IsHandshakeBudgetExhausted(int totalSleptMs)
{
    return totalSleptMs >= HANDSHAKE_TIMEOUT_MILLISECONDS;
}

static int MbedTlsStream_BioSend(void* ctx, const unsigned char* buf, size_t len)
{
    struct SolidSyslogMbedTlsStream* self = (struct SolidSyslogMbedTlsStream*) ctx;
    return SolidSyslogStream_Send(self->Config.Transport, buf, len) ? (int) len : -1;
}

/* Translate the non-blocking transport's Read contract into mbedTLS's BIO
 * recv contract:
 *   transport > 0 → bytes available, return the same positive count.
 *   transport = 0 → would-block. Must return MBEDTLS_ERR_SSL_WANT_READ so
 *                  the handshake retry loop polls; returning 0 or -1 would
 *                  abort the handshake on the first non-blocking read. */
static int MbedTlsStream_BioRecv(void* ctx, unsigned char* buf, size_t len)
{
    struct SolidSyslogMbedTlsStream* self = (struct SolidSyslogMbedTlsStream*) ctx;
    SolidSyslogSsize n = SolidSyslogStream_Read(self->Config.Transport, buf, len);
    int result = -1;
    if (n > 0)
    {
        result = (int) n;
    }
    else if (n == 0)
    {
        result = MBEDTLS_ERR_SSL_WANT_READ;
    }
    return result;
}

static inline bool MbedTlsStream_Send(struct SolidSyslogStream* base, const void* buffer, size_t size)
{
    struct SolidSyslogMbedTlsStream* self = MbedTlsStream_SelfFromBase(base);
    int rc = mbedtls_ssl_write(&self->SslContext, (const unsigned char*) buffer, size);
    return (rc > 0) && ((size_t) rc == size);
}

static inline SolidSyslogSsize MbedTlsStream_Read(struct SolidSyslogStream* base, void* buffer, size_t size)
{
    struct SolidSyslogMbedTlsStream* self = MbedTlsStream_SelfFromBase(base);
    int rc = mbedtls_ssl_read(&self->SslContext, (unsigned char*) buffer, size);
    SolidSyslogSsize result = -1;
    if (rc > 0)
    {
        result = (SolidSyslogSsize) rc;
    }
    else if (rc == MBEDTLS_ERR_SSL_WANT_READ)
    {
        result = 0;
    }
    return result;
}

static inline void MbedTlsStream_Close(struct SolidSyslogStream* base)
{
    struct SolidSyslogMbedTlsStream* self = MbedTlsStream_SelfFromBase(base);
    if (self->IsOpen)
    {
        (void) mbedtls_ssl_close_notify(&self->SslContext);
        mbedtls_ssl_free(&self->SslContext);
        mbedtls_ssl_config_free(&self->SslConfig);
        self->IsOpen = false;
    }
    SolidSyslogStream_Close(self->Config.Transport);
}
