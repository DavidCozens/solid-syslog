#include "SolidSyslogAtomicCounter.h"

#include <stdatomic.h>

struct SolidSyslogAtomicCounter
{
    atomic_uint_fast32_t value;
};

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(SolidSyslogAllocFunction alloc)
{
    struct SolidSyslogAtomicCounter* counter = alloc(sizeof(struct SolidSyslogAtomicCounter));
    if (counter != NULL)
    {
        atomic_init(&counter->value, 0);
    }
    return counter;
}

void SolidSyslogAtomicCounter_Destroy(struct SolidSyslogAtomicCounter* counter, SolidSyslogFreeFunction dealloc)
{
    dealloc(counter);
}

uint_fast32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* counter)
{
    return atomic_fetch_add(&counter->value, 1) + 1;
}
