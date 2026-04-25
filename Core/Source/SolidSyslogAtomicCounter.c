#include "SolidSyslogAtomicCounter.h"

#include "SolidSyslogAtomicOpsDefinition.h"

#include <stddef.h>

/* RFC 5424 §7.3.1: sequenceId MUST take values in [1, 2^31 - 1] and wrap to 1 (not 0) on overflow. */
static const uint32_t SEQUENCE_ID_MAX = 2147483647U;

struct SolidSyslogAtomicCounter
{
    struct SolidSyslogAtomicOps* ops;
};

static struct SolidSyslogAtomicCounter instance;

struct SolidSyslogAtomicCounter* SolidSyslogAtomicCounter_Create(struct SolidSyslogAtomicOps* ops)
{
    instance.ops = ops;
    return &instance;
}

void SolidSyslogAtomicCounter_Destroy(void)
{
    instance.ops = NULL;
}

uint32_t SolidSyslogAtomicCounter_Increment(struct SolidSyslogAtomicCounter* counter)
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
