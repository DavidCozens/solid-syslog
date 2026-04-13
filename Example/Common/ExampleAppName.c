#include "ExampleAppName.h"
#include "SolidSyslogFormatter.h"

#include <string.h>

static const char* appName;

void ExampleAppName_Set(const char* argv0)
{
    const char* slash = strrchr(argv0, '/');
    appName           = (slash != NULL) ? slash + 1 : argv0;
}

void ExampleAppName_Get(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_BoundedString(formatter, appName, strlen(appName));
}
