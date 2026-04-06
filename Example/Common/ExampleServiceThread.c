#include "ExampleServiceThread.h"
#include "SolidSyslog.h"

// cppcheck-suppress constParameter -- volatile bool written by another thread; const would be incorrect
// NOLINTNEXTLINE(readability-non-const-parameter) -- volatile bool written by another thread; const would be incorrect
void ExampleServiceThread_Run(volatile bool* shutdown)
{
    while (!(*shutdown))
    {
        SolidSyslog_Service();
    }

    while (SolidSyslog_Service())
    {
    }
}
