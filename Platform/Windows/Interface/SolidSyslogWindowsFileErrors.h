#ifndef SOLIDSYSLOGWINDOWSFILEERRORS_H
#define SOLIDSYSLOGWINDOWSFILEERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogWindowsFileErrors
    {
        WINDOWSFILE_ERROR_POOL_EXHAUSTED,
        WINDOWSFILE_ERROR_UNKNOWN_DESTROY,
        WINDOWSFILE_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource WindowsFileErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINDOWSFILEERRORS_H */
