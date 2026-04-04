#include "ExampleServiceThread.h"
#include "SolidSyslog.h"

// cppcheck-suppress constParameter -- volatile bool written by another thread; const would be incorrect
// NOLINTNEXTLINE(readability-non-const-parameter) -- volatile bool written by another thread; const would be incorrect
void ExampleServiceThread_Run(struct SolidSyslog* logger, volatile bool* shutdown)
{
    while (!(*shutdown))
    {
        SolidSyslog_Service(logger);
    }

    while (SolidSyslog_Service(logger))
    {
    }
}
