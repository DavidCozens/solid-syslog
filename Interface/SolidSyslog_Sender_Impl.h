#ifndef SOLIDSYSLOG_SENDER_IMPL_H
#define SOLIDSYSLOG_SENDER_IMPL_H

#include "SolidSyslog_Sender.h"

EXTERN_C_BEGIN

    struct SolidSyslog_Sender
    {
        void (*Send)(struct SolidSyslog_Sender* sender, const void* buffer, size_t size);
    };

EXTERN_C_END

#endif /* SOLIDSYSLOG_SENDER_IMPL_H */
