#ifndef SOLIDSYSLOGWINDOWSATOMICCOUNTERPRIVATE_H
#define SOLIDSYSLOGWINDOWSATOMICCOUNTERPRIVATE_H

#include <stdint.h>
#include <windows.h>

#include "SolidSyslogAtomicCounterDefinition.h"

struct SolidSyslogWindowsAtomicCounter
{
    struct SolidSyslogAtomicCounter Base;
    volatile LONG Value;
};

void WindowsAtomicCounter_Initialise(struct SolidSyslogAtomicCounter* base);
void WindowsAtomicCounter_Cleanup(struct SolidSyslogAtomicCounter* base);

/* Test-only helper — Tests/SolidSyslogWindowsAtomicCounterTestHelper.c
 * whitebox-includes this TU to seed the counter near INT32_MAX for the
 * wraparound contract test. Not part of the public API. */
void WindowsAtomicCounter_Init(struct SolidSyslogWindowsAtomicCounter* self, uint32_t value);

#endif /* SOLIDSYSLOGWINDOWSATOMICCOUNTERPRIVATE_H */
