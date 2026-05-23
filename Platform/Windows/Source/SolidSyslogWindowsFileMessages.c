#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogWindowsFileErrors.h"

static const char* WindowsFileError_AsString(uint8_t code)
{
    static const char* const messages[WINDOWSFILE_ERROR_MAX] = {
        [WINDOWSFILE_ERROR_POOL_EXHAUSTED] = "SolidSyslogWindowsFile_Create pool exhausted; returning fallback file",
        [WINDOWSFILE_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogWindowsFile_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) WINDOWSFILE_ERROR_MAX)
    {
        enum SolidSyslogWindowsFileErrors typed = (enum SolidSyslogWindowsFileErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource WindowsFileErrorSource = {"WindowsFile", WindowsFileError_AsString};
