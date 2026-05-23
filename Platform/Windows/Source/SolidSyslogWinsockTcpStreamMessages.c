#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWinsockTcpStreamErrors.h"

static const char* WinsockTcpStreamError_AsString(uint8_t code)
{
    static const char* const messages[WINSOCKTCPSTREAM_ERROR_MAX] = {
        [WINSOCKTCPSTREAM_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogWinsockTcpStream_Create pool exhausted; returning fallback stream",
        [WINSOCKTCPSTREAM_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWinsockTcpStream_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINSOCKTCPSTREAM_ERROR_MAX)
    {
        enum SolidSyslogWinsockTcpStreamErrors typed = (enum SolidSyslogWinsockTcpStreamErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WinsockTcpStreamErrorSource = {"WinsockTcpStream", WinsockTcpStreamError_AsString};
