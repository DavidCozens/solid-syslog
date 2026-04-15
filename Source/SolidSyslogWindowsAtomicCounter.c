#include "SolidSyslogAtomicCounter.h"

#include <windows.h>

struct SolidSyslogAtomicCounter
{
    volatile LONG value;
};

static struct SolidSyslogAtomicCounter instance;

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(void)
{
    instance.value = 0;
    return &instance;
}

void SolidSyslogAtomicCounter_Destroy(void)
{
    instance.value = 0;
}

uint_fast32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* counter)
{
    return (uint_fast32_t) InterlockedIncrement(&counter->value);
}
