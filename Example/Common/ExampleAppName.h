#ifndef EXAMPLEAPPNAME_H
#define EXAMPLEAPPNAME_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    void   ExampleAppName_Set(const char* argv0);
    size_t ExampleAppName_Get(char* buffer, size_t size);

EXTERN_C_END

#endif /* EXAMPLEAPPNAME_H */
