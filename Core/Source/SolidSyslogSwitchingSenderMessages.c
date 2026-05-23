#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogSwitchingSenderErrors.h"

static const char* SwitchingSenderError_AsString(uint8_t code)
{
    static const char* const messages[SWITCHINGSENDER_ERROR_MAX] = {
        [SWITCHINGSENDER_ERROR_NULL_CONFIG] = "SolidSyslogSwitchingSender_Create called with NULL config",
        [SWITCHINGSENDER_ERROR_NULL_SENDERS] = "SolidSyslogSwitchingSender_Create config.Senders is NULL",
        [SWITCHINGSENDER_ERROR_NULL_SELECTOR] = "SolidSyslogSwitchingSender_Create config.Selector is NULL",
        [SWITCHINGSENDER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogSwitchingSender_Create pool exhausted; returning fallback sender",
        [SWITCHINGSENDER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogSwitchingSender_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) SWITCHINGSENDER_ERROR_MAX)
    {
        enum SolidSyslogSwitchingSenderErrors typed = (enum SolidSyslogSwitchingSenderErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource SwitchingSenderErrorSource = {"SwitchingSender", SwitchingSenderError_AsString};
