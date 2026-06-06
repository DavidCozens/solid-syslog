#include "BddTargetLanguage.h"
#include "SolidSyslogSdValue.h"

static const char LANGUAGE_TAG[] = "en-GB";

void BddTargetLanguage_Get(struct SolidSyslogSdValue* value, void* context)
{
    (void) context;
    SolidSyslogSdValue_String(value, LANGUAGE_TAG);
}
