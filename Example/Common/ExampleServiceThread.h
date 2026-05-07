#ifndef EXAMPLESERVICETHREAD_H
#define EXAMPLESERVICETHREAD_H

#include "ExternC.h"
#include "SolidSyslogSleep.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    void ExampleServiceThread_Run(volatile bool* shutdown, SolidSyslogSleepFunction sleep);

EXTERN_C_END

#endif /* EXAMPLESERVICETHREAD_H */
