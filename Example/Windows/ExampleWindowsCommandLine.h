#ifndef EXAMPLEWINDOWSCOMMANDLINE_H
#define EXAMPLEWINDOWSCOMMANDLINE_H

#include "ExternC.h"
#include "SolidSyslogPrival.h"

EXTERN_C_BEGIN

    struct WindowsExampleOptions
    {
        enum SolidSyslog_Facility facility;
        enum SolidSyslog_Severity severity;
        const char*               messageId;
        const char*               msg;
    };

    /* Minimal CLI parser — recognises --facility N, --severity N, --msgid X,
       --message X, all optional. Unknown flags and flags missing their
       argument are silently ignored. Defaults match the SingleTask example:
       local0 / info / no msgid / no body. getopt is not available on MSVC
       and pulling in a vcpkg getopt for four flags would be overkill. */
    void ExampleWindowsCommandLine_Parse(int argc, char* argv[], struct WindowsExampleOptions* options);

EXTERN_C_END

#endif /* EXAMPLEWINDOWSCOMMANDLINE_H */
