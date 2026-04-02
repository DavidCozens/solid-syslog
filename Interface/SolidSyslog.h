#ifndef SOLIDSYSLOG_H
#define SOLIDSYSLOG_H

#include "SolidSyslogAlloc.h"
#include "SolidSyslogPrival.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;
    struct SolidSyslog;

    struct SolidSyslogConfig
    {
        struct SolidSyslogSender* sender;
        SolidSyslogAllocFn        alloc;
        SolidSyslogFreeFn         free;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);
    void                SolidSyslog_Log(struct SolidSyslog * logger, enum SolidSyslog_Facility facility, enum SolidSyslog_Severity severity);

EXTERN_C_END

#endif /* SOLIDSYSLOG_H */
