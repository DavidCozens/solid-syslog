#include "ExampleStderrErrorHandler.h"

#include <stdio.h>

#include "SolidSyslogError.h"
#include "SolidSyslogPrival.h"

static void StderrErrorHandler(void* context, enum SolidSyslog_Severity severity, const char* message)
{
    (void) context;
    (void) fprintf(stderr, "[solidsyslog] severity=%d %s\n", (int) severity, message);
}

void ExampleStderrErrorHandler_Install(void)
{
    SolidSyslog_SetErrorHandler(StderrErrorHandler, NULL);
}
