#ifndef SOLIDSYSLOGNULLSENDER_H
#define SOLIDSYSLOGNULLSENDER_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Send returns true, reporting the record as delivered so the Store drops it
     *  rather than accumulating undeliverables. Disconnect is a no-op. */
    struct SolidSyslogSender* SolidSyslogNullSender_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLSENDER_H */
