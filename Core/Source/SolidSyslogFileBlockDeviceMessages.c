#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogFileBlockDeviceErrors.h"

static const char* FileBlockDeviceError_AsString(uint8_t code)
{
    static const char* const messages[FILEBLOCKDEVICE_ERROR_MAX] = {
        [FILEBLOCKDEVICE_ERROR_POOL_EXHAUSTED] =
            "SolidSyslogFileBlockDevice_Create pool exhausted; returning fallback block device",
        [FILEBLOCKDEVICE_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogFileBlockDevice_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) FILEBLOCKDEVICE_ERROR_MAX)
    {
        enum SolidSyslogFileBlockDeviceErrors typed = (enum SolidSyslogFileBlockDeviceErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource FileBlockDeviceErrorSource = {"FileBlockDevice", FileBlockDeviceError_AsString};
