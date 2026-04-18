#include "ExampleAppName.h"
#include "SolidSyslogFormatter.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static const char* appName;
static size_t      appNameLength;

static bool EndsWithDotExe(const char* name, size_t length)
{
    /* Strips a trailing ".exe"/".EXE" so the syslog app name omits the
       Windows executable extension regardless of the build platform. */
    enum
    {
        EXE_SUFFIX_LENGTH = 4
    };

    if (length < EXE_SUFFIX_LENGTH)
    {
        return false;
    }

    const char* suffix = name + length - EXE_SUFFIX_LENGTH;
    return (suffix[0] == '.') && (tolower((unsigned char) suffix[1]) == 'e') && (tolower((unsigned char) suffix[2]) == 'x') &&
           (tolower((unsigned char) suffix[3]) == 'e');
}

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

    appName       = (separator != NULL) ? separator + 1 : argv0;
    appNameLength = strlen(appName);

    if (EndsWithDotExe(appName, appNameLength))
    {
        appNameLength -= 4;
    }
}

void ExampleAppName_Get(struct SolidSyslogFormatter* formatter)
{
    SolidSyslogFormatter_BoundedString(formatter, appName, appNameLength);
}
