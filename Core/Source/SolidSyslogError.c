#include "SolidSyslogError.h"

#include <stddef.h>

static void NoOpErrorHandler(void* context, enum SolidSyslog_Severity severity, const char* message)
{
    (void) context;
    (void) severity;
    (void) message;
}

static SolidSyslogErrorHandler currentHandler = NoOpErrorHandler;
static void*                   currentContext = NULL;

void SolidSyslog_SetErrorHandler(SolidSyslogErrorHandler handler, void* context)
{
    if (handler == NULL)
    {
        handler = NoOpErrorHandler;
    }
    currentHandler = handler;
    currentContext = context;
}

void SolidSyslog_Error(enum SolidSyslog_Severity severity, const char* message)
{
    currentHandler(currentContext, severity, message);
}
