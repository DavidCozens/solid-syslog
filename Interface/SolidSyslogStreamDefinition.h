#ifndef SOLIDSYSLOGSTREAMDEFINITION_H
#define SOLIDSYSLOGSTREAMDEFINITION_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream
    {
        bool (*Open)(struct SolidSyslogStream* self, const struct sockaddr_in* addr);
        bool (*Send)(struct SolidSyslogStream* self, const void* buffer, size_t size);
        /* void return: failure reporting deferred to Epic #31 (error handling) */
        void (*Close)(struct SolidSyslogStream* self);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAMDEFINITION_H */
