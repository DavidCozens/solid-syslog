#include "SolidSyslogMetaSd.h"
#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogStructuredDataDefinition.h"

#include <stdint.h>

struct SolidSyslogMetaSd
{
    struct SolidSyslogStructuredData base;
    struct SolidSyslogAtomicCounter* counter;
};

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter);

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

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter)
{
    struct SolidSyslogMetaSd* meta = (struct SolidSyslogMetaSd*) self;
    uint_fast32_t             id   = SolidSyslogAtomicCounter_Increment(meta->counter);

    SolidSyslogFormatter_BoundedString(formatter, "[meta sequenceId=\"", 18);
    SolidSyslogFormatter_Uint32(formatter, (uint32_t) id);
    SolidSyslogFormatter_BoundedString(formatter, "\"]", 2);
}
