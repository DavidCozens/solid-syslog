#ifndef SOLIDSYSLOGFORMATTER_H
#define SOLIDSYSLOGFORMATTER_H

#include "ExternC.h"

#include <stddef.h>
#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogFormatter
    {
        char*  buffer;
        size_t size;
        size_t position;
    };

    void   SolidSyslogFormatter_Create(struct SolidSyslogFormatter * formatter, char* buffer, size_t size);
    size_t SolidSyslogFormatter_Character(struct SolidSyslogFormatter * formatter, char value);
    size_t SolidSyslogFormatter_BoundedString(struct SolidSyslogFormatter * formatter, const char* source, size_t maxLength);
    size_t SolidSyslogFormatter_Uint32(struct SolidSyslogFormatter * formatter, uint32_t value);
    size_t SolidSyslogFormatter_PaddedUint32(struct SolidSyslogFormatter * formatter, uint32_t value, size_t width);
    size_t SolidSyslogFormatter_Remaining(struct SolidSyslogFormatter * formatter);

EXTERN_C_END

#endif /* SOLIDSYSLOGFORMATTER_H */
