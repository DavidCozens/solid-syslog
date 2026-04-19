#ifndef EXAMPLEINTERACTIVE_H
#define EXAMPLEINTERACTIVE_H

#include "ExternC.h"
#include "SolidSyslog.h"

#include <stdio.h>

EXTERN_C_BEGIN

    typedef void (*ExampleInteractiveSwitchHandler)(const char* name);

    void ExampleInteractive_Run(const struct SolidSyslogMessage* message, FILE* input, ExampleInteractiveSwitchHandler onSwitch);

EXTERN_C_END

#endif /* EXAMPLEINTERACTIVE_H */
