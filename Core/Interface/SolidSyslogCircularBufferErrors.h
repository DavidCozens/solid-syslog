/** @file
 *  Error codes and Source identity for the CircularBuffer. */
#ifndef SOLIDSYSLOGCIRCULARBUFFERERRORS_H
#define SOLIDSYSLOGCIRCULARBUFFERERRORS_H

#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource;

    /** Detail codes for events whose Source is CircularBufferErrorSource. A
     *  handler reads these off event->Detail after matching event->Source; the
     *  members name their own fault. */
    enum SolidSyslogCircularBufferErrors
    {
        CIRCULARBUFFER_ERROR_POOL_EXHAUSTED,
        CIRCULARBUFFER_ERROR_UNKNOWN_DESTROY,
        CIRCULARBUFFER_ERROR_MAX /**< One past the last code; never emitted. Bounds the range for iteration. */
    };

    /** The identity for events raised by a CircularBuffer. A handler matches by
     *  address (event->Source == &CircularBufferErrorSource), then reads
     *  event->Detail as an enum SolidSyslogCircularBufferErrors. */
    extern const struct SolidSyslogErrorSource CircularBufferErrorSource;

EXTERN_C_END

#endif /* SOLIDSYSLOGCIRCULARBUFFERERRORS_H */
