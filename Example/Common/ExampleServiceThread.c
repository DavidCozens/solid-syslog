#include "ExampleServiceThread.h"
#include "SolidSyslog.h"

void ExampleServiceThread_Run(struct SolidSyslog* logger, volatile bool* shutdown)
{
    while (!(*shutdown))
    {
        SolidSyslog_Service(logger);
    }

    while (SolidSyslog_Service(logger)) {}
}
