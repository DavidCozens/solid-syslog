#ifndef SOLIDSYSLOGSTDATOMICCOUNTERPRIVATE_H
#define SOLIDSYSLOGSTDATOMICCOUNTERPRIVATE_H

#include <stdatomic.h>
#include <stdint.h>

#include "SolidSyslogAtomicCounterDefinition.h"

struct SolidSyslogStdAtomicCounter
{
    struct SolidSyslogAtomicCounter Base;
    _Atomic uint32_t Value;
};

void StdAtomicCounter_Initialise(struct SolidSyslogAtomicCounter* base);
void StdAtomicCounter_Cleanup(struct SolidSyslogAtomicCounter* base);

/* Test-only helper — Tests/SolidSyslogStdAtomicCounterTestHelper.c
 * whitebox-includes this TU to seed the counter near INT32_MAX for the
 * wraparound contract test. Not part of the public API. */
void StdAtomicCounter_Init(struct SolidSyslogStdAtomicCounter* self, uint32_t value);

#endif /* SOLIDSYSLOGSTDATOMICCOUNTERPRIVATE_H */
