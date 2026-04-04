#include "ExampleCommandLine.h"

#include <getopt.h>
#include <stdlib.h>

int ExampleCommandLine_Parse(int argc, char* argv[], struct ExampleOptions* options)
{
    options->facility  = SOLIDSYSLOG_FACILITY_LOCAL0;
    options->severity  = SOLIDSYSLOG_SEVERITY_INFO;
    options->messageId = NULL;
    options->msg       = NULL;

    static struct option longOptions[] = {
        {"facility", required_argument, NULL, 'f'},
        {"severity", required_argument, NULL, 's'},
        {"msgid", required_argument, NULL, 'i'},
        {"message", required_argument, NULL, 'm'},
        {NULL, 0, NULL, 0},
    };

    int opt = 0;
    while ((opt = getopt_long(argc, argv, "f:s:i:m:", longOptions, NULL)) != -1)
    {
        switch (opt)
        {
            case 'f':
                options->facility = (enum SolidSyslog_Facility) atoi(optarg);
                break;
            case 's':
                options->severity = (enum SolidSyslog_Severity) atoi(optarg);
                break;
            case 'i':
                options->messageId = optarg;
                break;
            case 'm':
                options->msg = optarg;
                break;
            default:
                return 1;
        }
    }

    return 0;
}
