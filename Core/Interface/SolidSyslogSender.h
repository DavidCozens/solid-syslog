#ifndef SOLIDSYSLOGSENDER_H
#define SOLIDSYSLOGSENDER_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    bool SolidSyslogSender_Send(struct SolidSyslogSender * sender, const void* buffer, size_t size);
    void SolidSyslogSender_Disconnect(struct SolidSyslogSender * sender);

EXTERN_C_END

#endif /* SOLIDSYSLOGSENDER_H */
