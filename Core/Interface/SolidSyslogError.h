#ifndef SOLIDSYSLOGERROR_H
#define SOLIDSYSLOGERROR_H

#include <stdint.h>

#include "ExternC.h"
#include "SolidSyslogPrival.h"

/*
 * Policy severities for the universal lifecycle categories — one authoritative
 * level per category so the choice cannot drift across the dozens of emit
 * sites that raise them. See docs/error-severity.md. BAD_CONFIG is split: a
 * fatal misconfig (Create fell back to the Null object) uses the macro below,
 * while a degraded-but-delivering misconfig emits SOLIDSYSLOG_SEVERITY_WARNING
 * directly at the site — the two are genuinely different levels, so a single
 * shared macro would be a footgun.
 */
#define SOLIDSYSLOG_POOL_EXHAUSTED_SEVERITY SOLIDSYSLOG_SEVERITY_CRITICAL
#define SOLIDSYSLOG_BAD_CONFIG_FATAL_SEVERITY SOLIDSYSLOG_SEVERITY_CRITICAL
#define SOLIDSYSLOG_BAD_ARGUMENT_SEVERITY SOLIDSYSLOG_SEVERITY_CRITICAL
#define SOLIDSYSLOG_UNKNOWN_DESTROY_SEVERITY SOLIDSYSLOG_SEVERITY_WARNING

EXTERN_C_BEGIN

    struct SolidSyslogErrorSource
    {
        const char* Name;
    };

    struct SolidSyslogErrorEvent
    {
        enum SolidSyslogSeverity Severity;
        const struct SolidSyslogErrorSource* Source;
        uint16_t Category;
        int32_t Detail;
    };

    typedef void (*SolidSyslogErrorHandler)(void* context, const struct SolidSyslogErrorEvent* event);

    void SolidSyslog_SetErrorHandler(SolidSyslogErrorHandler handler, void* context);
    void SolidSyslog_Error(
        enum SolidSyslogSeverity severity,
        const struct SolidSyslogErrorSource* source,
        uint16_t category,
        int32_t detail
    );

EXTERN_C_END

#endif /* SOLIDSYSLOGERROR_H */
