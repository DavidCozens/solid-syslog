#ifndef SOLIDSYSLOGCONFIG_H
#define SOLIDSYSLOGCONFIG_H

#include "SolidSyslog.h"
#include "SolidSyslogAlloc.h"
#include "SolidSyslogTimestamp.h"

EXTERN_C_BEGIN

    struct SolidSyslogBuffer;
    struct SolidSyslogSender;
    struct SolidSyslogStructuredData;

    typedef size_t (*SolidSyslogStringFunction)(char* buffer, size_t size);

    struct SolidSyslogConfig
    {
        struct SolidSyslogBuffer*          buffer;
        struct SolidSyslogSender*          sender;
        SolidSyslogAllocFunction           alloc;
        SolidSyslogFreeFunction            free;
        SolidSyslogClockFunction           clock;
        SolidSyslogStringFunction          getHostname;
        SolidSyslogStringFunction          getAppName;
        SolidSyslogStringFunction          getProcId;
        struct SolidSyslogStructuredData** sd;
        size_t                             sdCount;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);

EXTERN_C_END

#endif /* SOLIDSYSLOGCONFIG_H */
