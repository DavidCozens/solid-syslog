#include <stdint.h>

#include "SolidSyslogCircularBufferErrors.h"
#include "SolidSyslogError.h"

static const char* CircularBufferError_AsString(uint8_t code)
{
    static const char* const messages[CIRCULARBUFFER_ERROR_MAX] = {
        [CIRCULARBUFFER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogCircularBuffer_Create pool exhausted; returning fallback buffer",
        [CIRCULARBUFFER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogCircularBuffer_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) CIRCULARBUFFER_ERROR_MAX)
    {
        enum SolidSyslogCircularBufferErrors typed = (enum SolidSyslogCircularBufferErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource CircularBufferErrorSource = {"CircularBuffer", CircularBufferError_AsString};
