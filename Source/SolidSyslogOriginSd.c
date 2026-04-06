#include "SolidSyslogOriginSd.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogStructuredDataDef.h"

enum
{
    ORIGIN_SOFTWARE_MAX  = 48,
    ORIGIN_SWVERSION_MAX = 32,
    ORIGIN_FORMATTED_MAX = 115
};

struct SolidSyslogOriginSd
{
    struct SolidSyslogStructuredData base;
    char                             formatted[ORIGIN_FORMATTED_MAX];
    size_t                           formattedLength;
};

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size);
static size_t FormatParam(char* buffer, const char* value, size_t remaining, size_t paramMax);
static size_t Min(size_t a, size_t b);

struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(SolidSyslogAllocFunction alloc, const char* software, const char* swVersion)
{
    if ((software == NULL) || (swVersion == NULL))
    {
        return NULL;
    }

    struct SolidSyslogOriginSd* instance = alloc(sizeof(struct SolidSyslogOriginSd));
    if (instance != NULL)
    {
        instance->base.Format = Format;
        size_t len            = 0;
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "[origin software=\"", ORIGIN_FORMATTED_MAX - len);
        len += FormatParam(instance->formatted + len, software, ORIGIN_FORMATTED_MAX - len, ORIGIN_SOFTWARE_MAX);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "\" swVersion=\"", ORIGIN_FORMATTED_MAX - len);
        len += FormatParam(instance->formatted + len, swVersion, ORIGIN_FORMATTED_MAX - len, ORIGIN_SWVERSION_MAX);
        len += SolidSyslogFormat_BoundedString(instance->formatted + len, "\"]", ORIGIN_FORMATTED_MAX - len);
        instance->formattedLength = len;
    }
    return &instance->base;
}

void SolidSyslogOriginSd_Destroy(struct SolidSyslogStructuredData* sd, SolidSyslogFreeFunction dealloc)
{
    dealloc(sd);
}

static size_t FormatParam(char* buffer, const char* value, size_t remaining, size_t paramMax)
{
    size_t limit = Min(remaining, paramMax + 1);
    return SolidSyslogFormat_BoundedString(buffer, value, limit);
}

static size_t Min(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size)
{
    struct SolidSyslogOriginSd* origin = (struct SolidSyslogOriginSd*) self;
    return SolidSyslogFormat_BoundedString(buffer, origin->formatted, size);
}
