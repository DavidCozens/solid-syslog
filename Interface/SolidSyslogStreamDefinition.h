#ifndef SOLIDSYSLOGSTREAMDEFINITION_H
#define SOLIDSYSLOGSTREAMDEFINITION_H

#include "SolidSyslogStream.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream
    {
        bool (*Open)(struct SolidSyslogStream* self, const struct SolidSyslogAddress* addr);
        bool (*Send)(struct SolidSyslogStream* self, const void* buffer, size_t size);
        SolidSyslogSsize (*Read)(struct SolidSyslogStream* self, void* buffer, size_t size);
        void (*Close)(struct SolidSyslogStream* self);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSTREAMDEFINITION_H */
