#include "SolidSyslogWindowsAtomicOps.h"

#include "SolidSyslogAtomicOpsDefinition.h"

#include <stddef.h>
#include <windows.h>

struct SolidSyslogWindowsAtomicOps
{
    struct SolidSyslogAtomicOps base;
    volatile LONG               value;
};

static uint32_t Load(struct SolidSyslogAtomicOps* self);
static bool     CompareAndSwap(struct SolidSyslogAtomicOps* self, uint32_t expected, uint32_t desired);

static struct SolidSyslogWindowsAtomicOps instance;

struct SolidSyslogAtomicOps* SolidSyslogWindowsAtomicOps_Create(void)
{
    instance.base.Load           = Load;
    instance.base.CompareAndSwap = CompareAndSwap;
    instance.value               = 0;
    return &instance.base;
}

void SolidSyslogWindowsAtomicOps_Destroy(void)
{
    instance.base.Load           = NULL;
    instance.base.CompareAndSwap = NULL;
    instance.value               = 0;
}

static uint32_t Load(struct SolidSyslogAtomicOps* self)
{
    struct SolidSyslogWindowsAtomicOps* windows = (struct SolidSyslogWindowsAtomicOps*) self;
    return (uint32_t) InterlockedCompareExchange(&windows->value, 0, 0);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) — CAS shape is universal (compare_exchange convention)
static bool CompareAndSwap(struct SolidSyslogAtomicOps* self, uint32_t expected, uint32_t desired)
{
    struct SolidSyslogWindowsAtomicOps* windows = (struct SolidSyslogWindowsAtomicOps*) self;
    LONG                                actual  = InterlockedCompareExchange(&windows->value, (LONG) desired, (LONG) expected);
    return (LONG) expected == actual;
}
