#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWinsockDatagramErrors.h"

static const char* WinsockDatagramError_AsString(uint8_t code)
{
    static const char* const messages[WINSOCKDATAGRAM_ERROR_MAX] = {
        [WINSOCKDATAGRAM_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogWinsockDatagram_Create pool exhausted; returning fallback datagram",
        [WINSOCKDATAGRAM_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWinsockDatagram_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINSOCKDATAGRAM_ERROR_MAX)
    {
        enum SolidSyslogWinsockDatagramErrors typed = (enum SolidSyslogWinsockDatagramErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WinsockDatagramErrorSource = {"WinsockDatagram", WinsockDatagramError_AsString};
