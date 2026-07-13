/** @file
 *  The no-op Datagram Null object: SendTo returns SENT (drops the datagram on the floor so
 *  the Store does not fill with undeliverables), MaxPayload returns the IPv6-safe default,
 *  Open and Close are no-ops. */
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
