#include "ErrorHandlerFakeEx.h"

#include <stddef.h>

#include "SolidSyslogError.h"

static int handleCallCount;
static enum SolidSyslogSeverity lastSeverity;
static const struct SolidSyslogErrorSource* lastSource;
static uint8_t lastCode;
static const void* lastContext;

static void Handle(
    void* context,
    enum SolidSyslogSeverity severity,
    const struct SolidSyslogErrorSource* source,
    uint8_t code
)
{
    handleCallCount++;
    lastSeverity = severity;
    lastSource = source;
    lastCode = code;
    lastContext = context;
}

void ErrorHandlerFakeEx_Install(void* context)
{
    handleCallCount = 0;
    lastSeverity = SOLIDSYSLOG_SEVERITY_DEBUG;
    lastSource = NULL;
    lastCode = 0U;
    lastContext = NULL;
    SolidSyslog_SetErrorHandlerEx(Handle, context);
}

int ErrorHandlerFakeEx_HandleCallCount(void)
{
    return handleCallCount;
}

enum SolidSyslogSeverity ErrorHandlerFakeEx_LastSeverity(void)
{
    return lastSeverity;
}

const struct SolidSyslogErrorSource* ErrorHandlerFakeEx_LastSource(void)
{
    return lastSource;
}

uint8_t ErrorHandlerFakeEx_LastCode(void)
{
    return lastCode;
}

const void* ErrorHandlerFakeEx_LastContext(void)
{
    return lastContext;
}
