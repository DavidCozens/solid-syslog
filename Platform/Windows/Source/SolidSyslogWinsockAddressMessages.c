#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWinsockAddressErrors.h"

static const char* WinsockAddressError_AsString(uint8_t code)
{
    static const char* const messages[WINSOCKADDRESS_ERROR_MAX] = {
        [WINSOCKADDRESS_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogWinsockAddress_Create pool exhausted; returning fallback address",
        [WINSOCKADDRESS_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWinsockAddress_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINSOCKADDRESS_ERROR_MAX)
    {
        enum SolidSyslogWinsockAddressErrors typed = (enum SolidSyslogWinsockAddressErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WinsockAddressErrorSource = {"WinsockAddress", WinsockAddressError_AsString};
