#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWindowsMutexErrors.h"

static const char* WindowsMutexError_AsString(uint8_t code)
{
    static const char* const messages[WINDOWSMUTEX_ERROR_MAX] = {
        [WINDOWSMUTEX_ERROR_POOL_EXHAUSTED] = "SolidSyslogWindowsMutex_Create pool exhausted; returning fallback mutex",
        [WINDOWSMUTEX_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWindowsMutex_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINDOWSMUTEX_ERROR_MAX)
    {
        enum SolidSyslogWindowsMutexErrors typed = (enum SolidSyslogWindowsMutexErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WindowsMutexErrorSource = {"WindowsMutex", WindowsMutexError_AsString};
