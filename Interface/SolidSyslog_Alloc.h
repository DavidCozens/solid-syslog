#ifndef SOLIDSYSLOG_ALLOC_H
#define SOLIDSYSLOG_ALLOC_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // NOLINTNEXTLINE(modernize-use-using) -- header is shared with C translation units; typedef required for C
    // compatibility
    typedef void* (*SolidSyslog_AllocFn)(size_t size);
    // NOLINTNEXTLINE(modernize-use-using) -- header is shared with C translation units; typedef required for C
    // compatibility
    typedef void (*SolidSyslog_FreeFn)(void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* SOLIDSYSLOG_ALLOC_H */
