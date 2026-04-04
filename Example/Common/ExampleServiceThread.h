#ifndef EXAMPLESERVICETHREAD_H
#define EXAMPLESERVICETHREAD_H

#include "ExternC.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    struct SolidSyslog;

    void ExampleServiceThread_Run(struct SolidSyslog* logger, volatile bool* shutdown);

EXTERN_C_END

#endif /* EXAMPLESERVICETHREAD_H */
