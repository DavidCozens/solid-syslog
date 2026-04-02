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

    struct SolidSyslogMessage
    {
        enum SolidSyslog_Facility facility;
        enum SolidSyslog_Severity severity;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);
    void                SolidSyslog_Log(struct SolidSyslog * logger, const struct SolidSyslogMessage* message);

EXTERN_C_END

#endif /* SOLIDSYSLOG_H */
