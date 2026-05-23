#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogFreeRtosMutexErrors.h"

static const char* FreeRtosMutexError_AsString(uint8_t code)
{
    static const char* const messages[FREERTOSMUTEX_ERROR_MAX] = {
        [FREERTOSMUTEX_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogFreeRtosMutex_Create pool exhausted; returning fallback mutex",
        [FREERTOSMUTEX_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogFreeRtosMutex_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) FREERTOSMUTEX_ERROR_MAX)
    {
        enum SolidSyslogFreeRtosMutexErrors typed = (enum SolidSyslogFreeRtosMutexErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource FreeRtosMutexErrorSource = {"FreeRtosMutex", FreeRtosMutexError_AsString};
