#ifndef SOLIDSYSLOGSDELEMENT_H
#define SOLIDSYSLOGSDELEMENT_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    /* The element writer handed to an SD's Format. Owns SD framing and the
     * SD-NAME / PARAM-NAME charset so an author cannot desync the framing.
     * Stack-transient, no pool (D.002). */
    struct SolidSyslogSdElement;

    /* Opens an SD-ELEMENT: emits "[name" for an IANA name (enterpriseNumber 0)
     * or "[name@enterpriseNumber" otherwise. */
    void SolidSyslogSdElement_Begin(struct SolidSyslogSdElement * element, const char* name, uint32_t enterpriseNumber);

EXTERN_C_END

#endif /* SOLIDSYSLOGSDELEMENT_H */
