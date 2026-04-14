#ifndef SOLIDSYSLOGFORMATTER_H
#define SOLIDSYSLOGFORMATTER_H

#include "ExternC.h"

#include <stddef.h>
#include <stdint.h>

EXTERN_C_BEGIN

    typedef size_t SolidSyslogFormatterStorage;

    enum
    {
        SOLIDSYSLOG_FORMATTER_OVERHEAD = 2
    };

/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) -- cannot compute array size as a constexpr in C */
#define SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(bufferSize) \
    (SOLIDSYSLOG_FORMATTER_OVERHEAD + (((bufferSize) + sizeof(SolidSyslogFormatterStorage) - 1) / sizeof(SolidSyslogFormatterStorage)))

    struct SolidSyslogFormatter;

    static inline struct SolidSyslogFormatter* SolidSyslogFormatter_FromStorage(SolidSyslogFormatterStorage * storage)
    {
        return (struct SolidSyslogFormatter*) storage; // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) -- C header; C++ cast not available
    }

    struct SolidSyslogFormatter* SolidSyslogFormatter_Create(SolidSyslogFormatterStorage * storage, size_t bufferSize);
    void                         SolidSyslogFormatter_Character(struct SolidSyslogFormatter * formatter, char value);
    void                         SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter * formatter, const char* source, size_t maxLength);
    void                         SolidSyslogFormatter_Uint32(struct SolidSyslogFormatter * formatter, uint32_t value);
    void                         SolidSyslogFormatter_TwoDigit(struct SolidSyslogFormatter * formatter, uint32_t value);
    void                         SolidSyslogFormatter_FourDigit(struct SolidSyslogFormatter * formatter, uint32_t value);
    void                         SolidSyslogFormatter_SixDigit(struct SolidSyslogFormatter * formatter, uint32_t value);
    const char*                  SolidSyslogFormatter_AsString(const struct SolidSyslogFormatter* formatter);
    size_t                       SolidSyslogFormatter_Length(const struct SolidSyslogFormatter* formatter);

EXTERN_C_END

#endif /* SOLIDSYSLOGFORMATTER_H */
