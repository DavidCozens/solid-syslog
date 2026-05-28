#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogLwipRawTcpStreamErrors.h"

static const char* LwipRawTcpStreamError_AsString(uint8_t code)
{
    static const char* const messages[LWIPRAWTCPSTREAM_ERROR_MAX] = {
        [LWIPRAWTCPSTREAM_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogLwipRawTcpStream_Create pool exhausted; returning fallback NullStream",
        [LWIPRAWTCPSTREAM_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogLwipRawTcpStream_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) LWIPRAWTCPSTREAM_ERROR_MAX)
    {
        enum SolidSyslogLwipRawTcpStreamErrors typed = (enum SolidSyslogLwipRawTcpStreamErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource LwipRawTcpStreamErrorSource = {"LwipRawTcpStream", LwipRawTcpStreamError_AsString};
