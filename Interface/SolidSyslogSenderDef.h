#ifndef SOLIDSYSLOGSENDERDEF_H
#define SOLIDSYSLOGSENDERDEF_H

#include "SolidSyslogSender.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender
    {
        void (*Send)(struct SolidSyslogSender* sender, const void* buffer, size_t size);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOGSENDERDEF_H */
