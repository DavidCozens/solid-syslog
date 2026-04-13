#include "SolidSyslogOriginSd.h"
#include "SolidSyslogFormatter.h"
#include "SolidSyslogStructuredDataDefinition.h"

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

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter);

static struct SolidSyslogOriginSd instance;

struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(const char* software, const char* swVersion)
{
    if ((software == NULL) || (swVersion == NULL))
    {
        return NULL;
    }

    struct SolidSyslogFormatter f;
    SolidSyslogFormatter_Create(&f, instance.formatted, ORIGIN_FORMATTED_MAX);

    instance.base.Format = Format;
    SolidSyslogFormatter_BoundedString(&f, "[origin software=\"", 18);
    SolidSyslogFormatter_BoundedString(&f, software, ORIGIN_SOFTWARE_MAX);
    SolidSyslogFormatter_BoundedString(&f, "\" swVersion=\"", 13);
    SolidSyslogFormatter_BoundedString(&f, swVersion, ORIGIN_SWVERSION_MAX);
    SolidSyslogFormatter_BoundedString(&f, "\"]", 2);
    instance.formattedLength = f.position;

    return &instance.base;
}

void SolidSyslogOriginSd_Destroy(void)
{
    instance.base.Format     = NULL;
    instance.formattedLength = 0;
    instance.formatted[0]    = '\0';
}

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter)
{
    struct SolidSyslogOriginSd* origin = (struct SolidSyslogOriginSd*) self;
    SolidSyslogFormatter_BoundedString(formatter, origin->formatted, origin->formattedLength);
}
