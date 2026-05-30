#ifndef SOLIDSYSLOGLWIPRAWDNSRESOLVERPRIVATE_H
#define SOLIDSYSLOGLWIPRAWDNSRESOLVERPRIVATE_H

#include <stdbool.h>

#include "SolidSyslogLwipRawDnsResolver.h"
#include "SolidSyslogResolverDefinition.h"
#include "lwip/ip_addr.h"

struct SolidSyslogLwipRawDnsResolver
{
    struct SolidSyslogResolver Base;
    struct SolidSyslogLwipRawDnsResolverConfig Config;
    /* In-flight async-resolve state. The bounded spin polls Done on the
     * caller's thread; the dns_found_callback writes Done + ResolvedOk +
     * ResolvedIp on the lwIP (tcpip) thread. Done is volatile because the two
     * threads are different — the synchronous ERR_OK path writes them too,
     * on the caller's thread, before any spin. */
    volatile bool Done;
    bool ResolvedOk;
    ip_addr_t ResolvedIp;
};

void LwipRawDnsResolver_Initialise(
    struct SolidSyslogResolver* base,
    const struct SolidSyslogLwipRawDnsResolverConfig* config
);
void LwipRawDnsResolver_Cleanup(struct SolidSyslogResolver* base);

#endif /* SOLIDSYSLOGLWIPRAWDNSRESOLVERPRIVATE_H */
