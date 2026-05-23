#ifndef ERRORHANDLERFAKE_H
#define ERRORHANDLERFAKE_H

#include <stdint.h>

#include "ExternC.h"
#include "SolidSyslogError.h"
#include "SolidSyslogPrival.h"

EXTERN_C_BEGIN

    void ErrorHandlerFake_Install(void* context);
    int ErrorHandlerFake_HandleCallCount(void);
    enum SolidSyslogSeverity ErrorHandlerFake_LastSeverity(void);
    const struct SolidSyslogErrorSource* ErrorHandlerFake_LastSource(void);
    uint8_t ErrorHandlerFake_LastCode(void);
    const void* ErrorHandlerFake_LastContext(void);

EXTERN_C_END

#endif /* ERRORHANDLERFAKE_H */
