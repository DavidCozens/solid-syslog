#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogFatFsFileErrors.h"

static const char* FatFsFileError_AsString(uint8_t code)
{
    static const char* const messages[FATFSFILE_ERROR_MAX] = {
        [FATFSFILE_ERROR_POOL_EXHAUSTED] = "SolidSyslogFatFsFile_Create pool exhausted; returning fallback file",
        [FATFSFILE_ERROR_UNKNOWN_DESTROY] = "SolidSyslogFatFsFile_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) FATFSFILE_ERROR_MAX)
    {
        enum SolidSyslogFatFsFileErrors typed = (enum SolidSyslogFatFsFileErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource FatFsFileErrorSource = {"FatFsFile", FatFsFileError_AsString};
