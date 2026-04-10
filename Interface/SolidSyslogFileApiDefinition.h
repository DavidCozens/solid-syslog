#ifndef SOLIDSYSLOGFILEAPIDEFINITION_H
#define SOLIDSYSLOGFILEAPIDEFINITION_H

#include "SolidSyslogFileApi.h"

EXTERN_C_BEGIN

    struct SolidSyslogFileApi
    {
        bool (*Open)(struct SolidSyslogFileApi* self, const char* path);
        void (*Close)(struct SolidSyslogFileApi* self);
        bool (*IsOpen)(struct SolidSyslogFileApi* self);
        bool (*Read)(struct SolidSyslogFileApi* self, void* buf, size_t count);
        bool (*Write)(struct SolidSyslogFileApi* self, const void* buf, size_t count);
        void (*SeekTo)(struct SolidSyslogFileApi* self, size_t offset);
        size_t (*Size)(struct SolidSyslogFileApi* self);
        void (*Truncate)(struct SolidSyslogFileApi* self);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGFILEAPIDEFINITION_H */
