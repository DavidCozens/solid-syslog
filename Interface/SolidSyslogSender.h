#ifndef SOLIDSYSLOGSENDER_H
#define SOLIDSYSLOGSENDER_H

#include "ExternC.h"
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    void SolidSyslogSender_Send(struct SolidSyslogSender * sender, const void* buffer, size_t size);

EXTERN_C_END

#endif /* SOLIDSYSLOGSENDER_H */
