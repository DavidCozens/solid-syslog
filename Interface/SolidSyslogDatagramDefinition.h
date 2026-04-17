#ifndef SOLIDSYSLOGDATAGRAMDEFINITION_H
#define SOLIDSYSLOGDATAGRAMDEFINITION_H

#include "SolidSyslogDatagram.h"

EXTERN_C_BEGIN

    struct SolidSyslogDatagram
    {
        /* void return: failure reporting deferred to Epic #31 (error handling) */
        void (*Open)(struct SolidSyslogDatagram* self);
        bool (*SendTo)(struct SolidSyslogDatagram* self, const void* buffer, size_t size, const struct sockaddr_in* addr);
        /* void return: failure reporting deferred to Epic #31 (error handling) */
        void (*Close)(struct SolidSyslogDatagram* self);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGDATAGRAMDEFINITION_H */
