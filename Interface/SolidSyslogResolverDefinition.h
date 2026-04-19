#ifndef SOLIDSYSLOGRESOLVERDEFINITION_H
#define SOLIDSYSLOGRESOLVERDEFINITION_H

#include "SolidSyslogResolver.h"
#include "SolidSyslogTransport.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogResolver
    {
        bool (*Resolve)(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, struct SolidSyslogAddress* result);
        bool (*ResolveAt)(struct SolidSyslogResolver* self, enum SolidSyslogTransport transport, const char* host, uint16_t port,
                          struct SolidSyslogAddress* result);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVERDEFINITION_H */
