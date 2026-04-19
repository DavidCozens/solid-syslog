#ifndef SOLIDSYSLOGSENDERDEFINITION_H
#define SOLIDSYSLOGSENDERDEFINITION_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender
    {
        bool (*Send)(struct SolidSyslogSender* sender, const void* buffer, size_t size);
        void (*Disconnect)(struct SolidSyslogSender* sender);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSENDERDEFINITION_H */
