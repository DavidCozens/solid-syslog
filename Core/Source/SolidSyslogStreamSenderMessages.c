#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogStreamSenderErrors.h"

static const char* StreamSenderError_AsString(uint8_t code)
{
    static const char* const messages[STREAMSENDER_ERROR_MAX] = {
        [STREAMSENDER_ERROR_NULL_CONFIG] = "SolidSyslogStreamSender_Create called with NULL config",
        [STREAMSENDER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogStreamSender_Create pool exhausted; returning fallback sender",
        [STREAMSENDER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogStreamSender_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) STREAMSENDER_ERROR_MAX)
    {
        enum SolidSyslogStreamSenderErrors typed = (enum SolidSyslogStreamSenderErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource StreamSenderErrorSource = {"StreamSender", StreamSenderError_AsString};
