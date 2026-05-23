#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogTimeQualitySdErrors.h"

static const char* TimeQualitySdError_AsString(uint8_t code)
{
    static const char* const messages[TIMEQUALITYSD_ERROR_MAX] = {
        [TIMEQUALITYSD_ERROR_NULL_CALLBACK] = "SolidSyslogTimeQualitySd_Create called with NULL getTimeQuality",
        [TIMEQUALITYSD_ERROR_POOL_EXHAUSTED] = "SolidSyslogTimeQualitySd_Create pool exhausted; returning fallback SD",
        [TIMEQUALITYSD_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogTimeQualitySd_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) TIMEQUALITYSD_ERROR_MAX)
    {
        enum SolidSyslogTimeQualitySdErrors typed = (enum SolidSyslogTimeQualitySdErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource TimeQualitySdErrorSource = {"TimeQualitySd", TimeQualitySdError_AsString};
