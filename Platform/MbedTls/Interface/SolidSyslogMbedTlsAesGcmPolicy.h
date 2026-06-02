#ifndef SOLIDSYSLOGMBEDTLSAESGCMPOLICY_H
#define SOLIDSYSLOGMBEDTLSAESGCMPOLICY_H

#include "ExternC.h"
#include "SolidSyslogKeyFunction.h"

struct SolidSyslogSecurityPolicy;
struct mbedtls_ctr_drbg_context;

EXTERN_C_BEGIN

    struct SolidSyslogMbedTlsAesGcmPolicyConfig
    {
        SolidSyslogKeyFunction GetKey; /* fetches the 32-byte AES-256 key on demand — required */
        void* KeyContext; /* passed through to GetKey; NULL is fine */
        /* Seeded CTR-DRBG the policy draws each record's 12-byte nonce from —
         * required. Caller owns it (same DI shape as SolidSyslogMbedTlsStream's
         * Rng); mbedTLS has no context-free RNG, so the nonce source is injected
         * rather than self-sourced as the OpenSSL sibling does with RAND_bytes. */
        struct mbedtls_ctr_drbg_context* Rng;
    };

    struct SolidSyslogSecurityPolicy* SolidSyslogMbedTlsAesGcmPolicy_Create(
        const struct SolidSyslogMbedTlsAesGcmPolicyConfig* config
    );
    void SolidSyslogMbedTlsAesGcmPolicy_Destroy(struct SolidSyslogSecurityPolicy * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGMBEDTLSAESGCMPOLICY_H */
