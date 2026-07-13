/** @file
 *  The POSIX SolidSyslogHeaderFieldFunction for RFC 5424 PROCID, for
 *  SolidSyslogConfig.GetProcessId. */
#ifndef SOLIDSYSLOGPOSIXPROCESSID_H
#define SOLIDSYSLOGPOSIXPROCESSID_H

#include "ExternC.h"

struct SolidSyslogHeaderField;

EXTERN_C_BEGIN

    /** Writes the process id (getpid) into @p field. @p context is unused. */
    void SolidSyslogPosixProcessId_Get(struct SolidSyslogHeaderField * field, void* context);

EXTERN_C_END

#endif /* SOLIDSYSLOGPOSIXPROCESSID_H */
