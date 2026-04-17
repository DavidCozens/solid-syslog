#ifndef SOLIDSYSLOGDATAGRAMDEFINITION_H
#define SOLIDSYSLOGDATAGRAMDEFINITION_H

#include "SolidSyslogDatagram.h"

EXTERN_C_BEGIN

    struct SolidSyslogDatagram
    {
        bool (*Open)(struct SolidSyslogDatagram* self);
        bool (*SendTo)(struct SolidSyslogDatagram* self, const void* buffer, size_t size, const struct SolidSyslogAddress* addr);
        void (*Close)(struct SolidSyslogDatagram* self);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGDATAGRAMDEFINITION_H */
