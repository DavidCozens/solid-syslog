#include "SolidSyslogError.h"

#include <stddef.h>

static void Error_NoOpErrorHandlerEx(
    void* context,
    enum SolidSyslogSeverity severity,
    const struct SolidSyslogErrorSource* source,
    uint8_t code
)
{
    (void) context;
    (void) severity;
    (void) source;
    (void) code;
}

static SolidSyslogErrorHandlerEx currentHandlerEx = Error_NoOpErrorHandlerEx;
static void* currentContextEx = NULL;

void SolidSyslog_SetErrorHandlerEx(SolidSyslogErrorHandlerEx handler, void* context)
{
    if (handler == NULL)
    {
        currentHandlerEx = Error_NoOpErrorHandlerEx;
    }
    else
    {
        currentHandlerEx = handler;
    }
    currentContextEx = context;
}

void SolidSyslog_ErrorEx(enum SolidSyslogSeverity severity, const struct SolidSyslogErrorSource* source, uint8_t code)
{
    currentHandlerEx(currentContextEx, severity, source, code);
}
