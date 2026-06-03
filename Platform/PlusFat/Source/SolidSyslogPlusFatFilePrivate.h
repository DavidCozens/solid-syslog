#ifndef SOLIDSYSLOGPLUSFATFILEPRIVATE_H
#define SOLIDSYSLOGPLUSFATFILEPRIVATE_H

#include <stdint.h>

#include "SolidSyslogError.h"
#include "SolidSyslogFileDefinition.h"
#include "SolidSyslogPlusFatFileErrors.h"
#include "SolidSyslogPrival.h"

struct SolidSyslogPlusFatFile
{
    struct SolidSyslogFile Base;
};

void PlusFatFile_Initialise(struct SolidSyslogFile* base);
void PlusFatFile_Cleanup(struct SolidSyslogFile* base);

static inline void PlusFatFile_Report(
    enum SolidSyslogSeverity severity,
    uint16_t category,
    enum SolidSyslogPlusFatFileErrors code
)
{
    SolidSyslog_Error(severity, &PlusFatFileErrorSource, category, (int32_t) code);
}

#endif /* SOLIDSYSLOGPLUSFATFILEPRIVATE_H */
