#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "SolidSyslogLwipRawDnsResolverPrivate.h"
#include "SolidSyslogNullResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "SolidSyslogTransport.h"

struct SolidSyslogAddress;

static bool LwipRawDnsResolver_Resolve(
    struct SolidSyslogResolver* base,
    enum SolidSyslogTransport transport,
    const char* host,
    uint16_t port,
    struct SolidSyslogAddress* result
);

void LwipRawDnsResolver_Initialise(
    struct SolidSyslogResolver* base,
    const struct SolidSyslogLwipRawDnsResolverConfig* config
)
{
    struct SolidSyslogLwipRawDnsResolver* self = (struct SolidSyslogLwipRawDnsResolver*) base;
    base->Resolve = LwipRawDnsResolver_Resolve;
    self->Config = *config;
    self->Done = false;
    self->ResolvedOk = false;
}

void LwipRawDnsResolver_Cleanup(struct SolidSyslogResolver* base)
{
    /* Overwrite the abstract base with the shared NullResolver vtable so
     * use-after-destroy resolves cleanly to a failed-lookup error path
     * rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullResolver_Get();
}

static bool LwipRawDnsResolver_Resolve(
    struct SolidSyslogResolver* base,
    enum SolidSyslogTransport transport,
    const char* host,
    uint16_t port,
    struct SolidSyslogAddress* result
)
{
    (void) base;
    (void) transport;
    (void) host;
    (void) port;
    (void) result;
    return false;
}
