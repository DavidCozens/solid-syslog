#include "SolidSyslogSdElementPrivate.h"

#include "SolidSyslogFormatter.h"

enum
{
    SDELEMENT_NAME_MAX = 32
};

void SolidSyslogSdElement_FromFormatter(struct SolidSyslogSdElement* element, struct SolidSyslogFormatter* formatter)
{
    element->Formatter = formatter;
}

void SolidSyslogSdElement_Begin(struct SolidSyslogSdElement* element, const char* name, uint32_t enterpriseNumber)
{
    (void) enterpriseNumber;
    SolidSyslogFormatter_AsciiCharacter(element->Formatter, '[');
    SolidSyslogFormatter_PrintUsAsciiString(element->Formatter, name, SDELEMENT_NAME_MAX);
}
