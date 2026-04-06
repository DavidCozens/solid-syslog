#include "SolidSyslogTimeQualitySd.h"
#include "SolidSyslogFormat.h"
#include "SolidSyslogStructuredDataDef.h"

struct SolidSyslogTimeQualitySd
{
    struct SolidSyslogStructuredData base;
    SolidSyslogTimeQualityFunction   getTimeQuality;
};

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size);
static size_t FormatBoolParam(char* buffer, size_t size, const char* name, bool value);
static size_t FormatSyncAccuracy(char* buffer, size_t size, uint32_t value);

static struct SolidSyslogTimeQualitySd instance;

struct SolidSyslogStructuredData* SolidSyslogTimeQualitySd_Create(SolidSyslogTimeQualityFunction getTimeQuality)
{
    instance.base.Format    = Format;
    instance.getTimeQuality = getTimeQuality;
    return &instance.base;
}

void SolidSyslogTimeQualitySd_Destroy(void)
{
    instance.base.Format    = NULL;
    instance.getTimeQuality = NULL;
}

static size_t Format(struct SolidSyslogStructuredData* self, char* buffer, size_t size)
{
    struct SolidSyslogTimeQualitySd* tq  = (struct SolidSyslogTimeQualitySd*) self;
    struct SolidSyslogTimeQuality    q   = {0};
    size_t                           len = 0;

    tq->getTimeQuality(&q);

    len += SolidSyslogFormat_BoundedString(buffer + len, "[timeQuality", size - len);
    len += FormatBoolParam(buffer + len, size - len, " tzKnown", q.tzKnown);
    len += FormatBoolParam(buffer + len, size - len, " isSynced", q.isSynced);
    len += FormatSyncAccuracy(buffer + len, size - len, q.syncAccuracyMicroseconds);
    len += SolidSyslogFormat_Character(buffer + len, ']');

    return len;
}

static size_t FormatBoolParam(char* buffer, size_t size, const char* name, bool value)
{
    size_t len = 0;

    len += SolidSyslogFormat_BoundedString(buffer + len, name, size - len);
    len += SolidSyslogFormat_Character(buffer + len, '=');
    len += SolidSyslogFormat_Character(buffer + len, '"');
    len += SolidSyslogFormat_Character(buffer + len, value ? '1' : '0');
    len += SolidSyslogFormat_Character(buffer + len, '"');

    return len;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters) -- size is the buffer limit, value is the accuracy to format; distinct semantics
static size_t FormatSyncAccuracy(char* buffer, size_t size, uint32_t value)
{
    if (value == SOLIDSYSLOG_SYNC_ACCURACY_OMIT)
    {
        return 0;
    }

    size_t len = 0;

    len += SolidSyslogFormat_BoundedString(buffer + len, " syncAccuracy=\"", size - len);
    len += SolidSyslogFormat_Uint32(buffer + len, value);
    len += SolidSyslogFormat_Character(buffer + len, '"');

    return len;
}
