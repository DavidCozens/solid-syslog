#ifndef SOLIDSYSLOG_SENDER_H
#define SOLIDSYSLOG_SENDER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct SolidSyslog_Sender
    {
        void (*Send)(struct SolidSyslog_Sender *sender, const void *buffer, size_t size);
    };

#ifdef __cplusplus
}
#endif

#endif /* SOLIDSYSLOG_SENDER_H */
