#ifndef SOLIDSYSLOGFATFSFILEERRORS_H
#define SOLIDSYSLOGFATFSFILEERRORS_H

#include "ExternC.h"
#include "SolidSyslogError.h"

EXTERN_C_BEGIN

    enum SolidSyslogFatFsFileErrors
    {
        FATFSFILE_ERROR_POOL_EXHAUSTED,
        FATFSFILE_ERROR_UNKNOWN_DESTROY,
        FATFSFILE_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource FatFsFileErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGFATFSFILEERRORS_H */
