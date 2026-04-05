#include "SolidSyslogMetaSd.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogStructuredDataDef.h"

#include <stdint.h>
#include <string.h>

struct SolidSyslogMetaSd
{
    struct SolidSyslogStructuredData  base;
    struct SolidSyslogAtomicCounter*  counter;
};

static size_t FormatUint32(char* buffer, uint32_t value);
static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size);

struct SolidSyslogStructuredData* SolidSyslogMetaSd_Create(SolidSyslogAllocFunction alloc, struct SolidSyslogAtomicCounter* counter)
{
    struct SolidSyslogMetaSd* instance = alloc(sizeof(struct SolidSyslogMetaSd));
    if (instance != NULL)
    {
        instance->base.Format = Format;
        instance->counter     = counter;
    }
    return &instance->base;
}

void SolidSyslogMetaSd_Destroy(struct SolidSyslogStructuredData* sd, SolidSyslogFreeFunction dealloc)
{
    dealloc(sd);
}

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size)
{
    struct SolidSyslogMetaSd* meta = (struct SolidSyslogMetaSd*) self;
    uint_fast32_t             id   = SolidSyslogAtomicCounter_Increment(meta->counter);

    const char* prefix = "[meta sequenceId=\"";
    const char* suffix = "\"]";
    size_t      len    = 0;

    size_t prefixLen = strlen(prefix);
    if (len + prefixLen < size)
    {
        memcpy(buffer + len, prefix, prefixLen);
        len += prefixLen;
    }

    len += FormatUint32(buffer + len, (uint32_t) id);

    size_t suffixLen = strlen(suffix);
    if (len + suffixLen < size)
    {
        memcpy(buffer + len, suffix, suffixLen);
        len += suffixLen;
    }

    buffer[len] = '\0';
    return len;
}

static size_t FormatUint32(char* buffer, uint32_t value)
{
    char   temp[11];
    size_t pos = 0;

    if (value == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }

    uint32_t remaining = value;
    while (remaining > 0)
    {
        temp[pos++] = (char) ('0' + (remaining % 10));
        remaining /= 10;
    }

    for (size_t i = 0; i < pos; i++)
    {
        buffer[i] = temp[pos - 1 - i];
    }
    buffer[pos] = '\0';
    return pos;
}
