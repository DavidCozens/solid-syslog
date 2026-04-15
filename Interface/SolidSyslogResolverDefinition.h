#ifndef SOLIDSYSLOGRESOLVERDEFINITION_H
#define SOLIDSYSLOGRESOLVERDEFINITION_H

#include "SolidSyslogResolver.h"
#include "SolidSyslogTransport.h"

EXTERN_C_BEGIN

    struct SolidSyslogResolver
    {
        void (*Resolve)(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct sockaddr_in* result);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVERDEFINITION_H */
