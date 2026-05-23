#include "BddTargetStderrErrorHandler.h"

#include <stdio.h>
#include <stdlib.h>

#include "SolidSyslogError.h"
#include "SolidSyslogPrival.h"

static void StderrErrorHandler(void* context, enum SolidSyslogSeverity severity, const char* message)
{
    (void) context;
    if (severity <= SOLIDSYSLOG_SEVERITY_ERROR)
    {
        (void) fprintf(stderr, "BDD-TARGET: FATAL: %s\n", message);
        (void) fflush(stderr);
        _Exit(3);
    }
    else
    {
        (void) fprintf(stderr, "[solidsyslog] severity=%d %s\n", (int) severity, message);
    }
}

static void StderrErrorHandlerEx(
    void* context,
    enum SolidSyslogSeverity severity,
    const struct SolidSyslogErrorSource* source,
    uint8_t code
)
{
    (void) context;
    const char* sourceName = "<unknown>";
    const char* message = "<no translation>";
    if (source != NULL)
    {
        sourceName = source->Name;
        if (source->AsString != NULL)
        {
            message = source->AsString(code);
        }
    }
    if (severity <= SOLIDSYSLOG_SEVERITY_ERROR)
    {
        (void) fprintf(stderr, "BDD-TARGET: FATAL: [%s/%u] %s\n", sourceName, (unsigned) code, message);
        (void) fflush(stderr);
        _Exit(3);
    }
    else
    {
        (void) fprintf(
            stderr,
            "[solidsyslog] severity=%d [%s/%u] %s\n",
            (int) severity,
            sourceName,
            (unsigned) code,
            message
        );
    }
}

void BddTargetStderrErrorHandler_Install(void)
{
    SolidSyslog_SetErrorHandler(StderrErrorHandler, NULL);
    SolidSyslog_SetErrorHandlerEx(StderrErrorHandlerEx, NULL);
}
