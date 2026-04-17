#ifndef SOLIDSYSLOGRESOLVERDEFINITION_H
#define SOLIDSYSLOGRESOLVERDEFINITION_H

#include "SolidSyslogResolver.h"
#include "SolidSyslogTransport.h"

EXTERN_C_BEGIN

    struct SolidSyslogResolver
    {
        bool (*Resolve)(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVERDEFINITION_H */
