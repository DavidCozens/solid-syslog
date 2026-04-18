#include "ExampleAppName.h"
#include "SolidSyslogFormatter.h"

#include <string.h>

static const char* appName;

void ExampleAppName_Set(const char* argv0)
{
    /* Avoid relational compare on a NULL pointer (UB in ISO C) by handling
       NULL separators explicitly before picking the rightmost. */
    const char* forwardSlash = strrchr(argv0, '/');
    const char* backSlash    = strrchr(argv0, '\\');
    const char* separator    = NULL;

    if (forwardSlash == NULL)
    {
        separator = backSlash;
    }
    else if (backSlash == NULL)
    {
        separator = forwardSlash;
    }
    else
    {
        separator = (forwardSlash > backSlash) ? forwardSlash : backSlash;
    }

    appName = (separator != NULL) ? separator + 1 : argv0;
}

void ExampleAppName_Get(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_BoundedString(formatter, appName, strlen(appName));
}
