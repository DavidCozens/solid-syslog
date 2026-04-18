#include "ExampleWindowsCommandLine.h"

#include <stdlib.h>
#include <string.h>

void ExampleWindowsCommandLine_Parse(int argc, char* argv[], struct WindowsExampleOptions* options)
{
    options->facility  = SOLIDSYSLOG_FACILITY_LOCAL0;
    options->severity  = SOLIDSYSLOG_SEVERITY_INFO;
    options->messageId = NULL;
    options->msg       = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (((i + 1) < argc) && (strcmp(argv[i], "--facility") == 0))
        {
            options->facility = (enum SolidSyslog_Facility) atoi(argv[++i]);
        }
        else if (((i + 1) < argc) && (strcmp(argv[i], "--severity") == 0))
        {
            options->severity = (enum SolidSyslog_Severity) atoi(argv[++i]);
        }
        else if (((i + 1) < argc) && (strcmp(argv[i], "--msgid") == 0))
        {
            options->messageId = argv[++i];
        }
        else if (((i + 1) < argc) && (strcmp(argv[i], "--message") == 0))
        {
            options->msg = argv[++i];
        }
    }
}
