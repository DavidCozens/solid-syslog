#ifndef SOLIDSYSLOGENDPOINT_H
#define SOLIDSYSLOGENDPOINT_H

#include "ExternC.h"
#include "SolidSyslogFormatter.h"

#include <stdint.h>

EXTERN_C_BEGIN

    enum
    {
        SOLIDSYSLOG_MAX_HOST_SIZE = 256
    };

    struct SolidSyslogEndpoint
    {
        struct SolidSyslogFormatter* host;    /* library-provided; user writes destination host into it */
        uint16_t                     port;    /* user assigns destination port */
        uint32_t                     version; /* user bumps on any change to host or port */
    };

    typedef void (*SolidSyslogEndpointFunction)(struct SolidSyslogEndpoint* endpoint);

EXTERN_C_END

#endif /* SOLIDSYSLOGENDPOINT_H */
