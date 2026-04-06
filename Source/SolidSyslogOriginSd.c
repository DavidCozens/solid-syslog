#include "SolidSyslogOriginSd.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogStructuredDataDef.h"

#include <string.h>

#define ORIGIN_SOFTWARE_MAX 48
#define ORIGIN_SWVERSION_MAX 32
#define ORIGIN_FORMATTED_MAX 115

struct SolidSyslogOriginSd
{
    struct SolidSyslogStructuredData base;
    char                             formatted[ORIGIN_FORMATTED_MAX];
    size_t                           formattedLength;
};

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size);

struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(SolidSyslogAllocFunction alloc, const char* software, const char* swVersion)
{
    struct SolidSyslogOriginSd* instance = alloc(sizeof(struct SolidSyslogOriginSd));
    if (instance != NULL)
    {
        instance->base.Format = Format;
        size_t len            = 0;
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "[origin software=\"", ORIGIN_FORMATTED_MAX - len);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, software, ORIGIN_FORMATTED_MAX - len);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "\" swVersion=\"", ORIGIN_FORMATTED_MAX - len);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, swVersion, ORIGIN_FORMATTED_MAX - len);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "\"]", ORIGIN_FORMATTED_MAX - len);
        instance->formattedLength = len;
    }
    return &instance->base;
}

void SolidSyslogOriginSd_Destroy(struct SolidSyslogStructuredData* sd, SolidSyslogFreeFunction dealloc)
{
    dealloc(sd);
}

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size)
{
    struct SolidSyslogOriginSd* origin = (struct SolidSyslogOriginSd*) self;
    return SolidSyslogFormat_BoundedString(buffer, origin->formatted, size);
}
