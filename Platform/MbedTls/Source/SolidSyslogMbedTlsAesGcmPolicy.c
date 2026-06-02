#include "SolidSyslogMbedTlsAesGcmPolicy.h"

#include <mbedtls/cipher.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/gcm.h>
#include <mbedtls/platform_util.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SolidSyslogMbedTlsAesGcmPolicyErrors.h"
#include "SolidSyslogMbedTlsAesGcmPolicyPrivate.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogSecurityPolicyDefinition.h"

enum
{
    AES_256_KEY_SIZE = 32,
    AES_256_KEY_BITS = 256,
    GCM_NONCE_SIZE = 12,
    GCM_TAG_SIZE = 16,
    /* Trailer is nonce ‖ tag — fits SOLIDSYSLOG_MAX_INTEGRITY_SIZE (32). */
    AES_GCM_TRAILER_SIZE = GCM_NONCE_SIZE + GCM_TAG_SIZE
};

static inline struct SolidSyslogMbedTlsAesGcmPolicy* MbedTlsAesGcmPolicy_SelfFromBase(
    struct SolidSyslogSecurityPolicy* base
);
static bool MbedTlsAesGcmPolicy_SealRecord(
    struct SolidSyslogSecurityPolicy* self,
    const struct SolidSyslogSecurityRecord* record
);
static bool MbedTlsAesGcmPolicy_OpenRecord(
    struct SolidSyslogSecurityPolicy* self,
    const struct SolidSyslogSecurityRecord* record
);
static bool MbedTlsAesGcmPolicy_FetchKey(struct SolidSyslogMbedTlsAesGcmPolicy* policy, uint8_t* keyOut);
static bool MbedTlsAesGcmPolicy_GcmEncrypt(
    const uint8_t* key,
    const uint8_t* nonce,
    uint8_t* body,
    uint16_t bodyLength,
    const uint8_t* aad,
    uint16_t aadLength,
    uint8_t* tagOut
);
static bool MbedTlsAesGcmPolicy_GcmDecrypt(
    const uint8_t* key,
    const uint8_t* nonce,
    uint8_t* body,
    uint16_t bodyLength,
    const uint8_t* aad,
    uint16_t aadLength,
    const uint8_t* tagIn
);

void MbedTlsAesGcmPolicy_Initialise(
    struct SolidSyslogSecurityPolicy* base,
    const struct SolidSyslogMbedTlsAesGcmPolicyConfig* config
)
{
    struct SolidSyslogMbedTlsAesGcmPolicy* self = MbedTlsAesGcmPolicy_SelfFromBase(base);
    self->Base.TrailerSize = AES_GCM_TRAILER_SIZE;
    self->Base.SealRecord = MbedTlsAesGcmPolicy_SealRecord;
    self->Base.OpenRecord = MbedTlsAesGcmPolicy_OpenRecord;
    self->Config = *config;
}

void MbedTlsAesGcmPolicy_Cleanup(struct SolidSyslogSecurityPolicy* base)
{
    /* No owned resources to release — the key is fetched on demand via the
     * GetKey callback and never stored on the instance, and the CTR-DRBG is
     * caller-owned. */
    (void) base;
}

static inline struct SolidSyslogMbedTlsAesGcmPolicy* MbedTlsAesGcmPolicy_SelfFromBase(
    struct SolidSyslogSecurityPolicy* base
)
{
    /* Base is the first member of the instance struct — see Private.h. */
    return (struct SolidSyslogMbedTlsAesGcmPolicy*) base;
}

/* Seals in place: the nonce occupies Trailer[0..GCM_NONCE_SIZE) and the tag the
 * remaining bytes. The header (Content[0..HeaderLength)) is authenticated as
 * associated data and left in clear; the body (the rest) is encrypted in place.
 * Fetches the key on demand and wipes it before returning. */
static bool MbedTlsAesGcmPolicy_SealRecord(
    struct SolidSyslogSecurityPolicy* self,
    const struct SolidSyslogSecurityRecord* record
)
{
    struct SolidSyslogMbedTlsAesGcmPolicy* policy = MbedTlsAesGcmPolicy_SelfFromBase(self);
    uint8_t* body = &record->Content[record->HeaderLength];
    uint16_t bodyLength = (uint16_t) (record->ContentLength - record->HeaderLength);
    uint8_t* nonce = record->Trailer;
    uint8_t* tag = &record->Trailer[GCM_NONCE_SIZE];

    bool sealed = false;
    uint8_t key[AES_256_KEY_SIZE];
    if (MbedTlsAesGcmPolicy_FetchKey(policy, key))
    {
        if (mbedtls_ctr_drbg_random(policy->Config.Rng, nonce, GCM_NONCE_SIZE) == 0)
        {
            if (MbedTlsAesGcmPolicy_GcmEncrypt(
                    key,
                    nonce,
                    body,
                    bodyLength,
                    record->Content,
                    record->HeaderLength,
                    tag
                ))
            {
                sealed = true;
            }
            else
            {
                MbedTlsAesGcmPolicy_Report(SOLIDSYSLOG_SEVERITY_ERROR, MBEDTLSAESGCMPOLICY_ERROR_ENCRYPT_FAILED);
            }
        }
        else
        {
            MbedTlsAesGcmPolicy_Report(SOLIDSYSLOG_SEVERITY_ERROR, MBEDTLSAESGCMPOLICY_ERROR_NONCE_FAILED);
        }
    }
    mbedtls_platform_zeroize(key, sizeof key);
    return sealed;
}

/* Fetches the AES-256 key on demand. Fails closed (and reports) if the key is
 * unavailable or not exactly 32 bytes — AES-256 admits no other key length. */
static bool MbedTlsAesGcmPolicy_FetchKey(struct SolidSyslogMbedTlsAesGcmPolicy* policy, uint8_t* keyOut)
{
    size_t keyLength = 0;
    bool fetched = policy->Config.GetKey(policy->Config.KeyContext, keyOut, AES_256_KEY_SIZE, &keyLength) &&
                   (keyLength == (size_t) AES_256_KEY_SIZE);
    if (!fetched)
    {
        MbedTlsAesGcmPolicy_Report(SOLIDSYSLOG_SEVERITY_ERROR, MBEDTLSAESGCMPOLICY_ERROR_KEY_UNAVAILABLE);
    }
    return fetched;
}

static bool MbedTlsAesGcmPolicy_GcmEncrypt(
    const uint8_t* key,
    const uint8_t* nonce,
    uint8_t* body,
    uint16_t bodyLength,
    const uint8_t* aad,
    uint16_t aadLength,
    uint8_t* tagOut
)
{
    /* One-shot AEAD: mbedTLS computes the whole tag in a single call, unlike
     * OpenSSL's incremental EVP chain. Body is encrypted in place (output ==
     * input is permitted for GCM encryption). */
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);
    bool ok = false;
    if (mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, AES_256_KEY_BITS) == 0)
    {
        int rc = mbedtls_gcm_crypt_and_tag(
            &ctx,
            MBEDTLS_GCM_ENCRYPT,
            bodyLength,
            nonce,
            GCM_NONCE_SIZE,
            aad,
            aadLength,
            body,
            body,
            GCM_TAG_SIZE,
            tagOut
        );
        ok = (rc == 0);
    }
    mbedtls_gcm_free(&ctx);
    return ok;
}

/* Opens in place: decrypts the body and verifies the tag over the header
 * (associated data) and ciphertext. Fetches the key on demand and wipes it. A
 * tag mismatch is the expected tamper-detected outcome and returns false
 * silently; only a genuine mbedTLS error is reported. */
static bool MbedTlsAesGcmPolicy_OpenRecord(
    struct SolidSyslogSecurityPolicy* self,
    const struct SolidSyslogSecurityRecord* record
)
{
    struct SolidSyslogMbedTlsAesGcmPolicy* policy = MbedTlsAesGcmPolicy_SelfFromBase(self);
    uint8_t* body = &record->Content[record->HeaderLength];
    uint16_t bodyLength = (uint16_t) (record->ContentLength - record->HeaderLength);
    const uint8_t* nonce = record->Trailer;
    const uint8_t* tag = &record->Trailer[GCM_NONCE_SIZE];

    bool opened = false;
    uint8_t key[AES_256_KEY_SIZE];
    if (MbedTlsAesGcmPolicy_FetchKey(policy, key))
    {
        opened =
            MbedTlsAesGcmPolicy_GcmDecrypt(key, nonce, body, bodyLength, record->Content, record->HeaderLength, tag);
    }
    mbedtls_platform_zeroize(key, sizeof key);
    return opened;
}

static bool MbedTlsAesGcmPolicy_GcmDecrypt(
    const uint8_t* key,
    const uint8_t* nonce,
    uint8_t* body,
    uint16_t bodyLength,
    const uint8_t* aad,
    uint16_t aadLength,
    const uint8_t* tagIn
)
{
    /* mbedtls_gcm_auth_decrypt verifies the tag itself and signals the verdict
     * through its return code: 0 = authentic, GCM_AUTH_FAILED = tamper/wrong key
     * (the expected rejection — return false silently, like the HMAC verify),
     * anything else = a genuine mbedTLS error worth reporting. */
    mbedtls_gcm_context ctx;
    mbedtls_gcm_init(&ctx);
    bool opened = false;
    bool errored = true;
    if (mbedtls_gcm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, AES_256_KEY_BITS) == 0)
    {
        int verdict = mbedtls_gcm_auth_decrypt(
            &ctx,
            bodyLength,
            nonce,
            GCM_NONCE_SIZE,
            aad,
            aadLength,
            tagIn,
            GCM_TAG_SIZE,
            body,
            body
        );
        if (verdict == 0)
        {
            opened = true;
            errored = false;
        }
        else if (verdict == MBEDTLS_ERR_GCM_AUTH_FAILED)
        {
            errored = false;
        }
        else
        {
            /* Genuine mbedTLS failure — errored stays true and is reported below. */
        }
    }
    mbedtls_gcm_free(&ctx);
    if (errored)
    {
        MbedTlsAesGcmPolicy_Report(SOLIDSYSLOG_SEVERITY_ERROR, MBEDTLSAESGCMPOLICY_ERROR_DECRYPT_FAILED);
    }
    return opened;
}
