#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogOriginSdErrors.h"

static const char* OriginSdError_AsString(uint8_t code)
{
    static const char* const messages[ORIGINSD_ERROR_MAX] = {
        [ORIGINSD_ERROR_POOL_EXHAUSTED] = "SolidSyslogOriginSd_Create pool exhausted; returning fallback SD",
        [ORIGINSD_ERROR_UNKNOWN_DESTROY] = "SolidSyslogOriginSd_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) ORIGINSD_ERROR_MAX)
    {
        enum SolidSyslogOriginSdErrors typed = (enum SolidSyslogOriginSdErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource OriginSdErrorSource = {"OriginSd", OriginSdError_AsString};
