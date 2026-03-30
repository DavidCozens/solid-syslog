#ifndef SOLIDSYSLOG_ALLOC_H
#define SOLIDSYSLOG_ALLOC_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    typedef void* (*SolidSyslog_AllocFn)(size_t size);
    typedef void (*SolidSyslog_FreeFn)(void* ptr);

EXTERN_C_END

#endif /* SOLIDSYSLOG_ALLOC_H */
