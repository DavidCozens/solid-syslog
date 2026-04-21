#ifndef SOLIDSYSLOGCRC16_H
#define SOLIDSYSLOGCRC16_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    uint16_t SolidSyslogCrc16_Compute(const uint8_t* data, uint16_t length);

EXTERN_C_END

#endif /* SOLIDSYSLOGCRC16_H */
