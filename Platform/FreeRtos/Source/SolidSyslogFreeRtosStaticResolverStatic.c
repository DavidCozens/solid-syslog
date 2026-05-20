#include "SolidSyslogFreeRtosStaticResolver.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogErrorMessages.h"
#include "SolidSyslogFreeRtosStaticResolverPrivate.h"
#include "SolidSyslogNullResolver.h"
#include "SolidSyslogPoolAllocator.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTunables.h"

struct SolidSyslogResolver;

static inline size_t FreeRtosStaticResolver_IndexFromHandle(const struct SolidSyslogResolver* base);
static inline void FreeRtosStaticResolver_CleanupAtIndex(size_t index, void* context);

static bool FreeRtosStaticResolver_InUse[SOLIDSYSLOG_FREE_RTOS_STATIC_RESOLVER_POOL_SIZE];
static struct SolidSyslogFreeRtosStaticResolver
    FreeRtosStaticResolver_Pool[SOLIDSYSLOG_FREE_RTOS_STATIC_RESOLVER_POOL_SIZE];
static struct SolidSyslogPoolAllocator FreeRtosStaticResolver_Allocator = {
    FreeRtosStaticResolver_InUse,
    SOLIDSYSLOG_FREE_RTOS_STATIC_RESOLVER_POOL_SIZE
};

struct SolidSyslogResolver* SolidSyslogFreeRtosStaticResolver_Create(const uint8_t ipv4Octets[4])
{
    size_t index = SolidSyslogPoolAllocator_AcquireFirstFree(&FreeRtosStaticResolver_Allocator);
    struct SolidSyslogResolver* handle = SolidSyslogNullResolver_Get();
    if (SolidSyslogPoolAllocator_IndexIsValid(&FreeRtosStaticResolver_Allocator, index))
    {
        FreeRtosStaticResolver_Initialise(&FreeRtosStaticResolver_Pool[index].Base, ipv4Octets);
        handle = &FreeRtosStaticResolver_Pool[index].Base;
    }
    else
    {
        SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERROR, SOLIDSYSLOG_ERROR_MSG_FREERTOSSTATICRESOLVER_POOL_EXHAUSTED);
    }
    return handle;
}

void SolidSyslogFreeRtosStaticResolver_Destroy(struct SolidSyslogResolver* base)
{
    size_t index = FreeRtosStaticResolver_IndexFromHandle(base);
    bool released = SolidSyslogPoolAllocator_IndexIsValid(&FreeRtosStaticResolver_Allocator, index) &&
                    SolidSyslogPoolAllocator_FreeIfInUse(
                        &FreeRtosStaticResolver_Allocator,
                        index,
                        FreeRtosStaticResolver_CleanupAtIndex,
                        NULL
                    );
    if (!released)
    {
        SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_WARNING, SOLIDSYSLOG_ERROR_MSG_FREERTOSSTATICRESOLVER_UNKNOWN_DESTROY);
    }
}

static inline size_t FreeRtosStaticResolver_IndexFromHandle(const struct SolidSyslogResolver* base)
{
    size_t result = SOLIDSYSLOG_FREE_RTOS_STATIC_RESOLVER_POOL_SIZE;
    for (size_t poolIndex = 0; poolIndex < SOLIDSYSLOG_FREE_RTOS_STATIC_RESOLVER_POOL_SIZE; poolIndex++)
    {
        if (base == &FreeRtosStaticResolver_Pool[poolIndex].Base)
        {
            result = poolIndex;
            break;
        }
    }
    return result;
}

static inline void FreeRtosStaticResolver_CleanupAtIndex(size_t index, void* context)
{
    (void) context;
    FreeRtosStaticResolver_Cleanup(&FreeRtosStaticResolver_Pool[index].Base);
}
