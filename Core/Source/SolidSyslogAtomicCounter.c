#include "SolidSyslogAtomicCounter.h"

#include "SolidSyslogAtomicCounterDefinition.h"

uint32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* base)
{
    return base->Increment(base);
}
