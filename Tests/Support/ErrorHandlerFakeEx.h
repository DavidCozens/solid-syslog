#ifndef ERRORHANDLERFAKEEX_H
#define ERRORHANDLERFAKEEX_H

#include <stdint.h>

#include "ExternC.h"
#include "SolidSyslogError.h"
#include "SolidSyslogPrival.h"

EXTERN_C_BEGIN

    void ErrorHandlerFakeEx_Install(void* context);
    int ErrorHandlerFakeEx_HandleCallCount(void);
    enum SolidSyslogSeverity ErrorHandlerFakeEx_LastSeverity(void);
    const struct SolidSyslogErrorSource* ErrorHandlerFakeEx_LastSource(void);
    uint8_t ErrorHandlerFakeEx_LastCode(void);
    const void* ErrorHandlerFakeEx_LastContext(void);

EXTERN_C_END

#endif /* ERRORHANDLERFAKEEX_H */
