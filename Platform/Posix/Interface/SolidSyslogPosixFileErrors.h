#ifndef SOLIDSYSLOGPOSIXFILEERRORS_H
#define SOLIDSYSLOGPOSIXFILEERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogPosixFileErrors
    {
        POSIXFILE_ERROR_POOL_EXHAUSTED,
        POSIXFILE_ERROR_UNKNOWN_DESTROY,
        POSIXFILE_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource PosixFileErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXFILEERRORS_H */
