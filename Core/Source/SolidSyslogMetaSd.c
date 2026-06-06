#include "SolidSyslogMetaSd.h"

#include <stddef.h>

#include "SolidSyslogAtomicCounter.h"
#include "SolidSyslogError.h"
#include "SolidSyslogMetaSdErrors.h"
#include "SolidSyslogMetaSdPrivate.h"
#include "SolidSyslogNullSd.h"
#include "SolidSyslogSdElementPrivate.h"
#include "SolidSyslogSdValue.h"
#include "SolidSyslogStructuredDataDefinition.h"

const struct SolidSyslogErrorSource MetaSdErrorSource = {"MetaSd"};

struct SolidSyslogFormatter;

static void MetaSd_Format(struct SolidSyslogStructuredData* base, struct SolidSyslogFormatter* formatter);

static inline struct SolidSyslogMetaSd* MetaSd_SelfFromBase(struct SolidSyslogStructuredData* base);

void MetaSd_Initialise(struct SolidSyslogStructuredData* base, const struct SolidSyslogMetaSdConfig* config)
{
    struct SolidSyslogMetaSd* self = MetaSd_SelfFromBase(base);
    self->Base.Format = MetaSd_Format;
    self->Counter = config->Counter;
    self->GetSysUpTime = config->GetSysUpTime;
    self->GetLanguage = config->GetLanguage;
    self->LanguageContext = config->LanguageContext;
}

void MetaSd_Cleanup(struct SolidSyslogStructuredData* base)
{
    /* Overwrite the abstract base with the shared NullSd vtable so use-after-destroy
     * is a safe no-op rather than a NULL-fn-pointer crash. Derived fields are private
     * to this TU; the next _Initialise overwrites them. */
    *base = *SolidSyslogNullSd_Get();
}

static void MetaSd_Format(struct SolidSyslogStructuredData* base, struct SolidSyslogFormatter* formatter)
{
    struct SolidSyslogMetaSd* self = MetaSd_SelfFromBase(base);
    struct SolidSyslogSdElement element;

    SolidSyslogSdElement_FromFormatter(&element, formatter);
    SolidSyslogSdElement_Begin(&element, "meta", 0U);
    SolidSyslogSdValue_Uint32(
        SolidSyslogSdElement_Param(&element, "sequenceId"),
        SolidSyslogAtomicCounter_Increment(self->Counter)
    );
    if (self->GetSysUpTime != NULL)
    {
        SolidSyslogSdValue_Uint32(SolidSyslogSdElement_Param(&element, "sysUpTime"), self->GetSysUpTime());
    }
    if (self->GetLanguage != NULL)
    {
        self->GetLanguage(SolidSyslogSdElement_Param(&element, "language"), self->LanguageContext);
    }
    SolidSyslogSdElement_End(&element);
}

static inline struct SolidSyslogMetaSd* MetaSd_SelfFromBase(struct SolidSyslogStructuredData* base)
{
    return (struct SolidSyslogMetaSd*) base;
}
