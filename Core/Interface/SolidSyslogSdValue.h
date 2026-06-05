#ifndef SOLIDSYSLOGSDVALUE_H
#define SOLIDSYSLOGSDVALUE_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /* The per-param value sink of the SD authoring API. A value producer is
     * handed only a SolidSyslogSdValue* — it can stream escaped, UTF-8-safe
     * content into the message buffer but cannot reach the raw formatter,
     * open a param, or break SD framing. Stack-transient, no pool (D.002). */
    struct SolidSyslogSdValue;

    /* Streams source (a NUL-terminated UTF-8 chunk) into the value, escaping
     * each of '"', '\\', ']' and substituting ill-formed UTF-8 with U+FFFD.
     * May be called repeatedly; a multi-byte codepoint split across two calls
     * is reassembled. Output is bounded by the message buffer. */
    void SolidSyslogSdValue_String(struct SolidSyslogSdValue * value, const char* source);

    /* Emits the decimal digits of number. Digits are never escapable, so no
     * escaping is applied. Output is bounded by the message buffer. */
    void SolidSyslogSdValue_Uint32(struct SolidSyslogSdValue * value, uint32_t number);

EXTERN_C_END

#endif /* SOLIDSYSLOGSDVALUE_H */
