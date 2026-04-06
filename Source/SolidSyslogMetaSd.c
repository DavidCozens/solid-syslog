#include "SolidSyslogMetaSd.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogStructuredDataDef.h"

#include <stdint.h>

struct SolidSyslogMetaSd
{
    struct SolidSyslogStructuredData base;
    struct SolidSyslogAtomicCounter* counter;
};

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size);

static struct SolidSyslogMetaSd instance;

struct SolidSyslogStructuredData* SolidSyslogMetaSd_Create(struct SolidSyslogAtomicCounter* counter)
{
    instance.base.Format = Format;
    instance.counter     = counter;
    return &instance.base;
}

void SolidSyslogMetaSd_Destroy(void)
{
    instance.base.Format = NULL;
    instance.counter     = NULL;
}

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size)
{
    struct SolidSyslogMetaSd* meta = (struct SolidSyslogMetaSd*) self;
    uint_fast32_t             id   = SolidSyslogAtomicCounter_Increment(meta->counter);
    size_t                    len  = 0;

    len += SolidSyslogFormat_BoundedString(buffer + len, "[meta sequenceId=\"", size - len);
    len += SolidSyslogFormat_Uint32(buffer + len, (uint32_t) id);
    len += SolidSyslogFormat_BoundedString(buffer + len, "\"]", size - len);

    return len;
}
