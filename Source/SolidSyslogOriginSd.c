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
    SolidSyslogFormatterStorage      formattedStorage[SOLIDSYSLOG_FORMATTER_STORAGE_SIZE(ORIGIN_FORMATTED_MAX)];
};

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter);

static struct SolidSyslogOriginSd instance;

struct SolidSyslogStructuredData* SolidSyslogOriginSd_Create(const char* software, const char* swVersion)
{
    if ((software == NULL) || (swVersion == NULL))
    {
        return NULL;
    }

    struct SolidSyslogFormatter* f = SolidSyslogFormatter_Create(instance.formattedStorage, ORIGIN_FORMATTED_MAX);

    instance.base.Format = Format;
    SolidSyslogFormatter_BoundedString(f, "[origin software=\"", 18);
    SolidSyslogFormatter_BoundedString(f, software, ORIGIN_SOFTWARE_MAX);
    SolidSyslogFormatter_BoundedString(f, "\" swVersion=\"", 13);
    SolidSyslogFormatter_BoundedString(f, swVersion, ORIGIN_SWVERSION_MAX);
    SolidSyslogFormatter_BoundedString(f, "\"]", 2);

    return &instance.base;
}

void SolidSyslogOriginSd_Destroy(void)
{
    instance.base.Format = NULL;
}

static void Format(struct SolidSyslogStructuredData* self, struct SolidSyslogFormatter* formatter)
{
    struct SolidSyslogOriginSd*  origin    = (struct SolidSyslogOriginSd*) self;
    struct SolidSyslogFormatter* preformat = SolidSyslogFormatter_FromStorage(origin->formattedStorage);

    SolidSyslogFormatter_BoundedString(formatter, SolidSyslogFormatter_Data(preformat), SolidSyslogFormatter_Length(preformat));
}
