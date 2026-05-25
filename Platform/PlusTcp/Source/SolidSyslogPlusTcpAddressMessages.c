#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogPlusTcpAddressErrors.h"

static const char* PlusTcpAddressError_AsString(uint8_t code)
{
    static const char* const messages[PLUSTCPADDRESS_ERROR_MAX] = {
        [PLUSTCPADDRESS_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogPlusTcpAddress_Create pool exhausted; returning fallback address",
        [PLUSTCPADDRESS_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogPlusTcpAddress_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) PLUSTCPADDRESS_ERROR_MAX)
    {
        enum SolidSyslogPlusTcpAddressErrors typed = (enum SolidSyslogPlusTcpAddressErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource PlusTcpAddressErrorSource = {"PlusTcpAddress", PlusTcpAddressError_AsString};
