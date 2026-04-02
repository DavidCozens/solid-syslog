#ifndef SOLIDSYSLOGCONFIG_H
#define SOLIDSYSLOGCONFIG_H

#include "SolidSyslog.h"
#include "SolidSyslogAlloc.h"
#include "SolidSyslogTimestamp.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    struct SolidSyslogConfig
    {
        struct SolidSyslogSender* sender;
        SolidSyslogAllocFn        alloc;
        SolidSyslogFreeFn         free;
        SolidSyslogClockFn        clock;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);

EXTERN_C_END

#endif /* SOLIDSYSLOGCONFIG_H */
