#include "ExampleServiceThread.h"
#include "SolidSyslog.h"

enum
{
    IDLE_YIELD_MILLISECONDS = 1
};

// cppcheck-suppress constParameter -- volatile bool written by another thread; const would be incorrect
// NOLINTNEXTLINE(readability-non-const-parameter) -- volatile bool written by another thread; const would be incorrect
void ExampleServiceThread_Run(volatile bool* shutdown, SolidSyslogSleepFunction sleep)
{
    while (!(*shutdown))
    {
        SolidSyslog_Service();
        sleep(IDLE_YIELD_MILLISECONDS);
    }
}
