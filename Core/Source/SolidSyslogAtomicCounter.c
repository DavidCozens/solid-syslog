#include "SolidSyslogAtomicCounter.h"

#include "SolidSyslogAtomicOpsDefinition.h"

#include <stdatomic.h>
#include <stddef.h>

struct SolidSyslogAtomicCounter
{
    atomic_uint_fast32_t         value;
    struct SolidSyslogAtomicOps* ops;
};

/* RFC 5424 §7.3.1: sequenceId MUST take values in [1, 2^31 - 1] and wrap to 1 (not 0) on overflow. */
static const uint32_t SEQUENCE_ID_MAX = 2147483647U;

static struct SolidSyslogAtomicCounter instance;

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(void)
{
    atomic_init(&instance.value, 0);
    instance.ops = NULL;
    return &instance;
}

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_CreateWithOps(struct SolidSyslogAtomicOps* ops)
{
    atomic_init(&instance.value, 0);
    instance.ops = ops;
    return &instance;
}

void SolidSyslogAtomicCounter_Destroy(void)
{
    atomic_init(&instance.value, 0);
    instance.ops = NULL;
}

uint_fast32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* counter)
{
    if (counter->ops != NULL)
    {
        uint32_t current = 0;
        uint32_t next    = 0;
        do
        {
            current = counter->ops->Load(counter->ops);
            next    = (current >= SEQUENCE_ID_MAX) ? 1U : current + 1U;
        } while (!counter->ops->CompareAndSwap(counter->ops, current, next));
        return next;
    }
    return atomic_fetch_add(&counter->value, 1) + 1;
}
