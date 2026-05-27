#include "SolidSyslogLwipRawTcpStreamPrivate.h"

#include <stdbool.h>
#include <stddef.h>

#include "SolidSyslogNullStream.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogStreamDefinition.h"

static inline struct SolidSyslogLwipRawTcpStream* LwipRawTcpStream_SelfFromBase(struct SolidSyslogStream* base);

void LwipRawTcpStream_Initialise(
    struct SolidSyslogStream* base,
    const struct SolidSyslogLwipRawTcpStreamConfig* config
)
{
    static const struct SolidSyslogLwipRawTcpStream DefaultLwipRawTcpStream = {0};

    struct SolidSyslogLwipRawTcpStream* self = LwipRawTcpStream_SelfFromBase(base);
    *self = DefaultLwipRawTcpStream;
    self->Config = *config;
}

static inline struct SolidSyslogLwipRawTcpStream* LwipRawTcpStream_SelfFromBase(struct SolidSyslogStream* base)
{
    return (struct SolidSyslogLwipRawTcpStream*) base;
}

void LwipRawTcpStream_Cleanup(struct SolidSyslogStream* base)
{
    /* Overwrite the abstract base with the shared NullStream vtable so
     * use-after-destroy is a safe no-op rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullStream_Get();
}
