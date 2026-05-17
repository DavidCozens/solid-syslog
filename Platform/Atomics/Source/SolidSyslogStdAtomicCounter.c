#include "SolidSyslogStdAtomicCounter.h"

#include "SolidSyslogMacros.h"

#include <stdatomic.h>
#include <stddef.h>

enum
{
    SEQUENCE_ID_MAX = 2147483647U /* RFC 5424 §7.3.1: values in [1, 2^31 - 1], wraps to 1 on overflow. */
};

struct SolidSyslogAtomicCounter
{
    _Atomic uint32_t Value;
};

SOLIDSYSLOG_STATIC_ASSERT(
    sizeof(struct SolidSyslogAtomicCounter) <= sizeof(SolidSyslogStdAtomicCounterStorage),
    SolidSyslogStdAtomicCounterStorage_too_small
);

static void StdAtomicCounter_Init(struct SolidSyslogAtomicCounter* self, uint32_t value);

struct SolidSyslogAtomicCounter* SolidSyslogStdAtomicCounter_Create(SolidSyslogStdAtomicCounterStorage* storage)
{
    struct SolidSyslogAtomicCounter* self = (struct SolidSyslogAtomicCounter*) storage;
    StdAtomicCounter_Init(self, 0U);
    return self;
}

uint32_t SolidSyslogStdAtomicCounter_Increment(struct SolidSyslogAtomicCounter* self)
{
    uint32_t current = atomic_load_explicit(&self->Value, memory_order_relaxed);
    uint32_t next = 0U;
    do
    {
        next = (current >= SEQUENCE_ID_MAX) ? 1U : (current + 1U);
    } while (!atomic_compare_exchange_strong_explicit(
        &self->Value, &current, next, memory_order_relaxed, memory_order_relaxed
    ));
    return next;
}

void SolidSyslogStdAtomicCounter_Destroy(struct SolidSyslogAtomicCounter* self)
{
    (void) self;
}

static void StdAtomicCounter_Init(struct SolidSyslogAtomicCounter* self, uint32_t value)
{
    atomic_init(&self->Value, value);
}
