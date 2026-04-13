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

static const char SD_SOFTWARE_PREFIX[] = "[origin software=\"";
static const char SD_VERSION_PREFIX[]  = "\" swVersion=\"";
static const char SD_SUFFIX[]          = "\"]";

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
    SolidSyslogFormatter_BoundedString(f, SD_SOFTWARE_PREFIX, sizeof(SD_SOFTWARE_PREFIX) - 1);
    SolidSyslogFormatter_BoundedString(f, software, ORIGIN_SOFTWARE_MAX);
    SolidSyslogFormatter_BoundedString(f, SD_VERSION_PREFIX, sizeof(SD_VERSION_PREFIX) - 1);
    SolidSyslogFormatter_BoundedString(f, swVersion, ORIGIN_SWVERSION_MAX);
    SolidSyslogFormatter_BoundedString(f, SD_SUFFIX, sizeof(SD_SUFFIX) - 1);

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
