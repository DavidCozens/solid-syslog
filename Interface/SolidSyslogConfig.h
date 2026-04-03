#ifndef SOLIDSYSLOGCONFIG_H
#define SOLIDSYSLOGCONFIG_H

#include "SolidSyslog.h"
#include "SolidSyslogAlloc.h"
#include "SolidSyslogTimestamp.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    typedef int (*SolidSyslogStringFunction)(char* buffer, size_t size);

    struct SolidSyslogConfig
    {
        struct SolidSyslogSender* sender;
        SolidSyslogAllocFunction  alloc;
        SolidSyslogFreeFunction   free;
        SolidSyslogClockFunction  clock;
        SolidSyslogStringFunction getHostname;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void                SolidSyslog_Destroy(struct SolidSyslog * logger);

EXTERN_C_END

#endif /* SOLIDSYSLOGCONFIG_H */
