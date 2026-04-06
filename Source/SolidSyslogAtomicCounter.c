#include "SolidSyslogAtomicCounter.h"

#include <stdatomic.h>

struct SolidSyslogAtomicCounter
{
    atomic_uint_fast32_t value;
};

static struct SolidSyslogAtomicCounter instance;

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(void)
{
    atomic_init(&instance.value, 0);
    return &instance;
}

void SolidSyslogAtomicCounter_Destroy(void)
{
    atomic_init(&instance.value, 0);
}

uint_fast32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* counter)
{
    return atomic_fetch_add(&counter->value, 1) + 1;
}
