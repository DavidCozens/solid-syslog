#ifndef SOLIDSYSLOGSDVALUEPRIVATE_H
#define SOLIDSYSLOGSDVALUEPRIVATE_H

#include "ExternC.h"

#include "SolidSyslogSdValue.h"

EXTERN_C_BEGIN

    struct SolidSyslogFormatter;

    /* Definition lives here (not the public header) so a producer handed a
     * SolidSyslogSdValue* cannot reach the wrapped formatter. SolidSyslogSdElement
     * (S14.02) embeds one of these and initialises it via _FromFormatter. */
    struct SolidSyslogSdValue
    {
        struct SolidSyslogFormatter* Formatter;
    };

    /* Internal constructor — wraps a message-buffer formatter so values stream
     * straight into it. Stack-transient: the caller owns the storage. */
    void SolidSyslogSdValue_FromFormatter(struct SolidSyslogSdValue * value, struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* SOLIDSYSLOGSDVALUEPRIVATE_H */
