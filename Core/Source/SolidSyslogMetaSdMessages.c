#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogMetaSdErrors.h"

static const char* MetaSdError_AsString(uint8_t code)
{
    static const char* const messages[METASD_ERROR_MAX] = {
        [METASD_ERROR_NULL_CONFIG] = "SolidSyslogMetaSd_Create called with NULL config",
        [METASD_ERROR_NULL_COUNTER] = "SolidSyslogMetaSd_Create config.Counter is NULL",
        [METASD_ERROR_POOL_EXHAUSTED] = "SolidSyslogMetaSd_Create pool exhausted; returning fallback SD",
        [METASD_ERROR_UNKNOWN_DESTROY] = "SolidSyslogMetaSd_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) METASD_ERROR_MAX)
    {
        enum SolidSyslogMetaSdErrors typed = (enum SolidSyslogMetaSdErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource MetaSdErrorSource = {"MetaSd", MetaSdError_AsString};
