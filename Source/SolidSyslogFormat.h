#ifndef SOLIDSYSLOGFORMAT_H
#define SOLIDSYSLOGFORMAT_H

#include "ExternC.h"
#include <stddef.h>
#include <stdint.h>

EXTERN_C_BEGIN

    size_t SolidSyslogFormat_Character(char* buffer, char value);
    size_t SolidSyslogFormat_BoundedString(char* buffer, const char* source, size_t bufferSize);
    size_t SolidSyslogFormat_Nilvalue(char* buffer);
    size_t SolidSyslogFormat_Uint32(char* buffer, uint32_t value);

EXTERN_C_END

#endif /* SOLIDSYSLOGFORMAT_H */
