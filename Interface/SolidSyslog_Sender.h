#ifndef SOLIDSYSLOG_SENDER_H
#define SOLIDSYSLOG_SENDER_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslog_Sender;

    void SolidSyslog_Sender_Send(struct SolidSyslog_Sender * sender, const void* buffer, size_t size);

EXTERN_C_END

#endif /* SOLIDSYSLOG_SENDER_H */
