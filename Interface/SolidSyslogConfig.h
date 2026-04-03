#ifndef SOLIDSYSLOGCONFIG_H
#define SOLIDSYSLOGCONFIG_H

#include "SolidSyslog.h"
#include "SolidSyslogAlloc.h"
#include "SolidSyslogTimestamp.h"

EXTERN_C_BEGIN

    struct SolidSyslogBuffer;

    typedef size_t (*SolidSyslogStringFunction)(char* buffer, size_t size);

    struct SolidSyslogConfig
    {
        struct SolidSyslogBuffer* buffer;
        SolidSyslogAllocFunction  alloc;
        SolidSyslogFreeFunction   free;
        SolidSyslogClockFunction  clock;
        SolidSyslogStringFunction getHostname;
        SolidSyslogStringFunction getAppName;
        SolidSyslogStringFunction getProcId;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);

EXTERN_C_END

#endif /* SOLIDSYSLOGCONFIG_H */
