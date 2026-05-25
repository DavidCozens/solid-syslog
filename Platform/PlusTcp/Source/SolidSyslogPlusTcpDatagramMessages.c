#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogPlusTcpDatagramErrors.h"

static const char* PlusTcpDatagramError_AsString(uint8_t code)
{
    static const char* const messages[PLUSTCPDATAGRAM_ERROR_MAX] = {
        [PLUSTCPDATAGRAM_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogPlusTcpDatagram_Create pool exhausted; returning fallback datagram",
        [PLUSTCPDATAGRAM_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogPlusTcpDatagram_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) PLUSTCPDATAGRAM_ERROR_MAX)
    {
        enum SolidSyslogPlusTcpDatagramErrors typed = (enum SolidSyslogPlusTcpDatagramErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource PlusTcpDatagramErrorSource = {"PlusTcpDatagram", PlusTcpDatagramError_AsString};
