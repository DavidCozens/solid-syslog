#ifndef SOLIDSYSLOGSDELEMENTPRIVATE_H
#define SOLIDSYSLOGSDELEMENTPRIVATE_H

#include "ExternC.h"

#include "SolidSyslogSdElement.h"

EXTERN_C_BEGIN

    struct SolidSyslogFormatter;

    /* Definition lives here (not the public header) so an SD author handed a
     * SolidSyslogSdElement* cannot reach the wrapped formatter. */
    struct SolidSyslogSdElement
    {
        struct SolidSyslogFormatter* Formatter;
    };

    /* Internal constructor — wraps a message-buffer formatter. MessageFormatter
     * (S14.06) builds one of these around the handed formatter and passes it to
     * each SD's Format. Stack-transient: the caller owns the storage. */
    void SolidSyslogSdElement_FromFormatter(struct SolidSyslogSdElement * element, struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* SOLIDSYSLOGSDELEMENTPRIVATE_H */
