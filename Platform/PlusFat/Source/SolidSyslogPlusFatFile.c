#include "SolidSyslogPlusFatFile.h"

#include "SolidSyslogFileDefinition.h"
#include "SolidSyslogNullFile.h"
#include "SolidSyslogPlusFatFileErrors.h"
#include "SolidSyslogPlusFatFilePrivate.h"

const struct SolidSyslogErrorSource PlusFatFileErrorSource = {"PlusFatFile"};

void PlusFatFile_Initialise(struct SolidSyslogFile* base)
{
    /* Vtable wiring is grown by the behavioural TDD slices. */
    (void) base;
}

void PlusFatFile_Cleanup(struct SolidSyslogFile* base)
{
    /* Overwrite the abstract base with the shared NullFile vtable so
     * use-after-destroy is a safe no-op rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullFile_Get();
}
