#include "SolidSyslogMbedTlsHmacSha256Policy.h"

#include <stdbool.h>
#include <stdint.h>

#include "SolidSyslogMbedTlsHmacSha256PolicyPrivate.h"
#include "SolidSyslogSecurityPolicyDefinition.h"

enum
{
    HMAC_SHA256_TAG_SIZE = 32
};

static inline struct SolidSyslogMbedTlsHmacSha256Policy* MbedTlsHmacSha256Policy_SelfFromBase(
    struct SolidSyslogSecurityPolicy* base
);
static bool MbedTlsHmacSha256Policy_ComputeIntegrity(
    struct SolidSyslogSecurityPolicy* self,
    const uint8_t* data,
    uint16_t length,
    uint8_t* integrityOut
);
static bool MbedTlsHmacSha256Policy_VerifyIntegrity(
    struct SolidSyslogSecurityPolicy* self,
    const uint8_t* data,
    uint16_t length,
    const uint8_t* integrityIn
);

void MbedTlsHmacSha256Policy_Initialise(
    struct SolidSyslogSecurityPolicy* base,
    const struct SolidSyslogMbedTlsHmacSha256PolicyConfig* config
)
{
    struct SolidSyslogMbedTlsHmacSha256Policy* self = MbedTlsHmacSha256Policy_SelfFromBase(base);
    self->Base.IntegritySize = HMAC_SHA256_TAG_SIZE;
    self->Base.ComputeIntegrity = MbedTlsHmacSha256Policy_ComputeIntegrity;
    self->Base.VerifyIntegrity = MbedTlsHmacSha256Policy_VerifyIntegrity;
    self->Config = *config;
}

void MbedTlsHmacSha256Policy_Cleanup(struct SolidSyslogSecurityPolicy* base)
{
    /* No owned resources to release — the key is fetched on demand via the
     * GetKey callback and never stored on the instance. */
    (void) base;
}

static inline struct SolidSyslogMbedTlsHmacSha256Policy* MbedTlsHmacSha256Policy_SelfFromBase(
    struct SolidSyslogSecurityPolicy* base
)
{
    /* Base is the first member of the instance struct — see Private.h. */
    return (struct SolidSyslogMbedTlsHmacSha256Policy*) base;
}

/* Slice 4 wires these to mbedtls_md_hmac (SHA-256), fetching the key via
 * self->Config.GetKey into a transient buffer and wiping it after use. Until
 * then they are no-ops so the lifecycle / pool behaviour can be pinned down
 * independently. */
static bool MbedTlsHmacSha256Policy_ComputeIntegrity(
    struct SolidSyslogSecurityPolicy* self,
    const uint8_t* data,
    uint16_t length,
    // NOLINTNEXTLINE(readability-non-const-parameter) -- integrityOut is non-const to match the vtable signature
    uint8_t* integrityOut
)
{
    (void) self;
    (void) data;
    (void) length;
    (void) integrityOut;
    return true;
}

static bool MbedTlsHmacSha256Policy_VerifyIntegrity(
    struct SolidSyslogSecurityPolicy* self,
    const uint8_t* data,
    uint16_t length,
    const uint8_t* integrityIn
)
{
    (void) self;
    (void) data;
    (void) length;
    (void) integrityIn;
    return true;
}
