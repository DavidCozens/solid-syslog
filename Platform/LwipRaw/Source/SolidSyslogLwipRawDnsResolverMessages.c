#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogLwipRawDnsResolverErrors.h"

static const char* LwipRawDnsResolverError_AsString(uint8_t code)
{
    static const char* const messages[LWIPRAWDNSRESOLVER_ERROR_MAX] = {
        [LWIPRAWDNSRESOLVER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogLwipRawDnsResolver_Create pool exhausted; returning fallback NullResolver",
        [LWIPRAWDNSRESOLVER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogLwipRawDnsResolver_Destroy called with a handle not issued by this pool",
        [LWIPRAWDNSRESOLVER_ERROR_RESOLVE_TIMEOUT] =
            "SolidSyslogLwipRawDnsResolver_Resolve timed out waiting for dns_gethostbyname; reporting failed lookup",
    };
    const char* result = "unknown";
    if (code < (uint8_t) LWIPRAWDNSRESOLVER_ERROR_MAX)
    {
        enum SolidSyslogLwipRawDnsResolverErrors typed = (enum SolidSyslogLwipRawDnsResolverErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource LwipRawDnsResolverErrorSource = {
    "LwipRawDnsResolver",
    LwipRawDnsResolverError_AsString
};
