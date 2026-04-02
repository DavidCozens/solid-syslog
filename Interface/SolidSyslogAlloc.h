#ifndef SOLIDSYSLOGALLOC_H
#define SOLIDSYSLOGALLOC_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    typedef void* (*SolidSyslogAllocFunction)(size_t size);
    typedef void (*SolidSyslogFreeFunction)(void* ptr);

EXTERN_C_END

#endif /* SOLIDSYSLOGALLOC_H */
