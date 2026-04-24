#include "StringFake.h"
#include "SolidSyslogFormatter.h"

#include <string.h>

static const char* fakeHostname;
static const char* fakeAppName;
static const char* fakeProcessId;

void StringFake_Reset(void)
{
    fakeHostname  = "";
    fakeAppName   = "";
    fakeProcessId = "";
}

void StringFake_SetHostname(const char* hostname)
{
    fakeHostname = hostname;
}

void StringFake_GetHostname(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_RawBoundedString(formatter, fakeHostname, strlen(fakeHostname));
}

void StringFake_SetAppName(const char* appName)
{
    fakeAppName = appName;
}

void StringFake_GetAppName(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_RawBoundedString(formatter, fakeAppName, strlen(fakeAppName));
}

void StringFake_SetProcessId(const char* procId)
{
    fakeProcessId = procId;
}

void StringFake_GetProcessId(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_RawBoundedString(formatter, fakeProcessId, strlen(fakeProcessId));
}
