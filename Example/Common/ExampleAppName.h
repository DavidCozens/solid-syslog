#ifndef EXAMPLEAPPNAME_H
#define EXAMPLEAPPNAME_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogFormatter;

    void ExampleAppName_Set(const char* argv0);
    void ExampleAppName_Get(struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* EXAMPLEAPPNAME_H */
