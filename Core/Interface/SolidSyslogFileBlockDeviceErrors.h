#ifndef SOLIDSYSLOGFILEBLOCKDEVICEERRORS_H
#define SOLIDSYSLOGFILEBLOCKDEVICEERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    enum SolidSyslogFileBlockDeviceErrors
    {
        FILEBLOCKDEVICE_ERROR_POOL_EXHAUSTED,
        FILEBLOCKDEVICE_ERROR_UNKNOWN_DESTROY,
        FILEBLOCKDEVICE_ERROR_MAX
    };

    extern const struct SolidSyslogErrorSource FileBlockDeviceErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGFILEBLOCKDEVICEERRORS_H */
