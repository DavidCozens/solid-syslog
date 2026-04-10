#ifndef EXAMPLECOMMANDLINE_H
#define EXAMPLECOMMANDLINE_H

#include "ExternC.h"
#include "SolidSyslogPrival.h"

EXTERN_C_BEGIN

    struct ExampleOptions
    {
        enum SolidSyslog_Facility facility;
        enum SolidSyslog_Severity severity;
        const char*               messageId;
        const char*               msg;
        const char*               transport;
    };

    int ExampleCommandLine_Parse(int argc, char* argv[], struct ExampleOptions* options);

EXTERN_C_END

#endif /* EXAMPLECOMMANDLINE_H */
