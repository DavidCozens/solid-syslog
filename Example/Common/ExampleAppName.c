#include "ExampleAppName.h"
#include "SolidSyslogFormatter.h"

#include <string.h>

static const char* appName;

void ExampleAppName_Set(const char* argv0)
{
    const char* forwardSlash = strrchr(argv0, '/');
    const char* backSlash    = strrchr(argv0, '\\');
    const char* separator    = (forwardSlash > backSlash) ? forwardSlash : backSlash;
    appName                  = (separator != NULL) ? separator + 1 : argv0;
}

void ExampleAppName_Get(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_BoundedString(formatter, appName, strlen(appName));
}
