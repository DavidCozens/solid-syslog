#ifndef SOLIDSYSLOG_ALLOC_H
#define SOLIDSYSLOG_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void* (*SolidSyslog_AllocFn)(size_t size);
    typedef void (*SolidSyslog_FreeFn)(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* SOLIDSYSLOG_ALLOC_H */
