#ifndef SOLIDSYSLOGCONFIG_H
#define SOLIDSYSLOGCONFIG_H

#include "SolidSyslog.h"
#include "SolidSyslogTimestamp.h"

#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogBuffer;
    struct SolidSyslogSender;
    struct SolidSyslogStore;
    struct SolidSyslogStructuredData;

    typedef size_t (*SolidSyslogStringFunction)(char* buffer, size_t size);

    struct SolidSyslogConfig
    {
        struct SolidSyslogBuffer*          buffer;
        struct SolidSyslogSender*          sender;
        SolidSyslogClockFunction           clock;
        SolidSyslogStringFunction          getHostname;
        SolidSyslogStringFunction          getAppName;
        SolidSyslogStringFunction          getProcId;
        struct SolidSyslogStore*           store;
        struct SolidSyslogStructuredData** sd;
        size_t                             sdCount;
    };

    void SolidSyslog_Create(const struct SolidSyslogConfig* config);
    void SolidSyslog_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGCONFIG_H */
