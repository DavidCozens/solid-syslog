#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogPassthroughBufferErrors.h"

static const char* PassthroughBufferError_AsString(uint8_t code)
{
    static const char* const messages[PASSTHROUGHBUFFER_ERROR_MAX] = {
        [PASSTHROUGHBUFFER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogPassthroughBuffer_Create pool exhausted; returning fallback buffer",
        [PASSTHROUGHBUFFER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogPassthroughBuffer_Destroy called with a handle not issued by this pool",
        [PASSTHROUGHBUFFER_ERROR_NULL_SENDER] =
            "SolidSyslogPassthroughBuffer_Create called with NULL sender; returning fallback buffer",
    };
    const char* result = "unknown";
    if (code < (uint8_t) PASSTHROUGHBUFFER_ERROR_MAX)
    {
        enum SolidSyslogPassthroughBufferErrors typed = (enum SolidSyslogPassthroughBufferErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource PassthroughBufferErrorSource = {
    "PassthroughBuffer",
    PassthroughBufferError_AsString
};
