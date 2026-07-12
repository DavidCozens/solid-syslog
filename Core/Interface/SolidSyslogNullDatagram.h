#ifndef SOLIDSYSLOGNULLDATAGRAM_H
#define SOLIDSYSLOGNULLDATAGRAM_H

#include "ExternC.h"

EXTERN_C_BEGIN

    /** Open and Close are no-ops. SendTo returns SENT, reporting the datagram as
     *  delivered so the Store drops it rather than accumulating undeliverables.
     *  MaxPayload returns the IPv6-safe default. */
    struct SolidSyslogDatagram* SolidSyslogNullDatagram_Get(void);

EXTERN_C_END

#endif /* SOLIDSYSLOGNULLDATAGRAM_H */
