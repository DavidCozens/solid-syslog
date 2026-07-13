#ifndef SOLIDSYSLOGRESOLVERDEFINITION_H
#define SOLIDSYSLOGRESOLVERDEFINITION_H

#include <stdint.h>
#include <stdbool.h>

#include "SolidSyslogTransport.h"
#include "ExternC.h"

struct SolidSyslogAddress;

EXTERN_C_BEGIN

    /** Extension point for resolving a destination host to a platform address.
     *  Implementors write @p result (caller-owned SolidSyslogAddress storage,
     *  downcast to the platform's concrete address) only on a true return, and
     *  return false on any failure so the caller's unresolved-host path runs.
     *  Honouring @p transport and @p host is best-effort: a fixed-destination
     *  resolver may ignore either. A lookup that touches the network may block. */
    struct SolidSyslogResolver
    {
        bool (*Resolve)(
            struct SolidSyslogResolver* base,
            enum SolidSyslogTransport transport,
            const char* host,
            uint16_t port,
            struct SolidSyslogAddress* result
        );
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGRESOLVERDEFINITION_H */
