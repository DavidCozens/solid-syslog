#ifndef SOLIDSYSLOG_H
#define SOLIDSYSLOG_H

#include "ExternC.h"
#include "SolidSyslogPrival.h"

#include <stdbool.h>

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_MAX_MESSAGE_SIZE = 512
    };

    struct SolidSyslog;

    struct SolidSyslogMessage
    {
        enum SolidSyslog_Facility facility;
        enum SolidSyslog_Severity severity;
        const char*               messageId;
        const char*               msg;
    };

    void SolidSyslog_Log(struct SolidSyslog * logger, const struct SolidSyslogMessage* message);
    bool SolidSyslog_Service(struct SolidSyslog * logger);

EXTERN_C_END

#endif /* SOLIDSYSLOG_H */
