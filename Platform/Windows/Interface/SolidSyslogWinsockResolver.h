#ifndef SOLIDSYSLOGWINSOCKRESOLVERH
#define SOLIDSYSLOGWINSOCKRESOLVERH

#include "SolidSyslogResolver.h"

EXTERN_C_BEGIN

    /* Precondition: caller has invoked WSAStartup() before using the resolver,
       and will call WSACleanup() on shutdown. The library does not manage
       Winsock lifecycle. */
    struct SolidSyslogResolver* SolidSyslogWinsockResolver_Create(const char* (*getHost)(void), // NOLINT(modernize-redundant-void-arg) -- C idiom
                                                                  int (*getPort)(void));        // NOLINT(modernize-redundant-void-arg) -- C idiom
    void                        SolidSyslogWinsockResolver_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGWINSOCKRESOLVERH */
