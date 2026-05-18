#include "SolidSyslogCircularBuffer.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SolidSyslogBufferDefinition.h"
#include "SolidSyslogCircularBufferPrivate.h"
#include "SolidSyslogError.h"
#include "SolidSyslogErrorMessages.h"
#include "SolidSyslogPoolAllocator.h"
#include "SolidSyslogPrival.h"
#include "SolidSyslogTunables.h"

struct SolidSyslogMutex;

static bool Fallback_Read(struct SolidSyslogBuffer* base, void* data, size_t maxSize, size_t* bytesRead);
static void Fallback_Write(struct SolidSyslogBuffer* base, const void* data, size_t size);
static size_t CircularBuffer_IndexFromHandle(const struct SolidSyslogBuffer* base);
static void CircularBuffer_CleanupAtIndex(size_t index, void* context);

static bool InUse[SOLIDSYSLOG_CIRCULAR_BUFFER_POOL_SIZE];
static struct SolidSyslogCircularBuffer Pool[SOLIDSYSLOG_CIRCULAR_BUFFER_POOL_SIZE];
static struct SolidSyslogBuffer Fallback = {Fallback_Write, Fallback_Read};
static struct SolidSyslogPoolAllocator Allocator = {InUse, SOLIDSYSLOG_CIRCULAR_BUFFER_POOL_SIZE};

struct SolidSyslogBuffer* SolidSyslogCircularBuffer_Create(
    struct SolidSyslogMutex* mutex,
    uint8_t* ring,
    size_t ringBytes
)
{
    size_t index = SolidSyslogPoolAllocator_AcquireFirstFree(&Allocator);
    struct SolidSyslogBuffer* handle = &Fallback;
    if (SolidSyslogPoolAllocator_IndexIsValid(&Allocator, index))
    {
        CircularBuffer_Initialise(&Pool[index].Base, mutex, ring, ringBytes);
        handle = &Pool[index].Base;
    }
    else
    {
        SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERROR, SOLIDSYSLOG_ERROR_MSG_CIRCULARBUFFER_POOL_EXHAUSTED);
    }
    return handle;
}

void SolidSyslogCircularBuffer_Destroy(struct SolidSyslogBuffer* base)
{
    size_t index = CircularBuffer_IndexFromHandle(base);
    bool released = SolidSyslogPoolAllocator_IndexIsValid(&Allocator, index) &&
                    SolidSyslogPoolAllocator_FreeIfInUse(&Allocator, index, CircularBuffer_CleanupAtIndex, NULL);
    if (!released)
    {
        SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_WARNING, SOLIDSYSLOG_ERROR_MSG_CIRCULARBUFFER_UNKNOWN_DESTROY);
    }
}

static size_t CircularBuffer_IndexFromHandle(const struct SolidSyslogBuffer* base)
{
    size_t result = SOLIDSYSLOG_CIRCULAR_BUFFER_POOL_SIZE;
    for (size_t poolIndex = 0; poolIndex < SOLIDSYSLOG_CIRCULAR_BUFFER_POOL_SIZE; poolIndex++)
    {
        if (base == &Pool[poolIndex].Base)
        {
            result = poolIndex;
            break;
        }
    }
    return result;
}

static void CircularBuffer_CleanupAtIndex(size_t index, void* context)
{
    (void) context;
    CircularBuffer_Cleanup(&Pool[index].Base);
}

static bool Fallback_Read(struct SolidSyslogBuffer* base, void* data, size_t maxSize, size_t* bytesRead)
{
    (void) base;
    (void) data;
    (void) maxSize;
    *bytesRead = 0;
    return false;
}

static void Fallback_Write(struct SolidSyslogBuffer* base, const void* data, size_t size)
{
    (void) base;
    (void) data;
    (void) size;
}
