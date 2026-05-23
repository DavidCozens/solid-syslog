#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWindowsAtomicCounterErrors.h"

static const char* WindowsAtomicCounterError_AsString(uint8_t code)
{
    static const char* const messages[WINDOWSATOMICCOUNTER_ERROR_MAX] = {
        [WINDOWSATOMICCOUNTER_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogWindowsAtomicCounter_Create pool exhausted; returning fallback counter",
        [WINDOWSATOMICCOUNTER_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWindowsAtomicCounter_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINDOWSATOMICCOUNTER_ERROR_MAX)
    {
        enum SolidSyslogWindowsAtomicCounterErrors typed = (enum SolidSyslogWindowsAtomicCounterErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WindowsAtomicCounterErrorSource = {
    "WindowsAtomicCounter",
    WindowsAtomicCounterError_AsString
};
