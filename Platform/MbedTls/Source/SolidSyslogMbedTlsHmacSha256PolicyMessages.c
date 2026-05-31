#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogMbedTlsHmacSha256PolicyErrors.h"

static const char* MbedTlsHmacSha256PolicyError_AsString(uint8_t code)
{
    static const char* const messages[MBEDTLSHMACSHA256POLICY_ERROR_MAX] = {
        [MBEDTLSHMACSHA256POLICY_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogMbedTlsHmacSha256Policy_Create pool exhausted; returning fallback policy",
        [MBEDTLSHMACSHA256POLICY_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogMbedTlsHmacSha256Policy_Destroy called with a handle not issued by this pool",
        [MBEDTLSHMACSHA256POLICY_ERROR_BAD_CONFIG] =
            "SolidSyslogMbedTlsHmacSha256Policy_Create given a NULL config or NULL GetKey; returning fallback policy",
    };
    const char* result = "unknown";
    if (code < (uint8_t) MBEDTLSHMACSHA256POLICY_ERROR_MAX)
    {
        enum SolidSyslogMbedTlsHmacSha256PolicyErrors typed = (enum SolidSyslogMbedTlsHmacSha256PolicyErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource MbedTlsHmacSha256PolicyErrorSource = {
    "MbedTlsHmacSha256Policy",
    MbedTlsHmacSha256PolicyError_AsString
};
