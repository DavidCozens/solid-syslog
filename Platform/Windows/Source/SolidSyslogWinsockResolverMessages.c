#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWinsockResolverErrors.h"

static const char* WinsockResolverError_AsString(uint8_t code)
{
    static const char* const messages[WINSOCKRESOLVER_ERROR_MAX] = {
        [WINSOCKRESOLVER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogWinsockResolver_Create pool exhausted; returning fallback resolver",
        [WINSOCKRESOLVER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWinsockResolver_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINSOCKRESOLVER_ERROR_MAX)
    {
        enum SolidSyslogWinsockResolverErrors typed = (enum SolidSyslogWinsockResolverErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WinsockResolverErrorSource = {"WinsockResolver", WinsockResolverError_AsString};
