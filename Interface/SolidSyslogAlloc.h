#ifndef SOLIDSYSLOGALLOC_H
#define SOLIDSYSLOGALLOC_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    typedef void* (*SolidSyslogAllocFn)(size_t size);
    typedef void (*SolidSyslogFreeFn)(void* ptr);

EXTERN_C_END

#endif /* SOLIDSYSLOGALLOC_H */
