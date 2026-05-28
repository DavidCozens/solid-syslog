#ifndef SOLIDSYSLOGLWIPRAWTCPSTREAM_H
#define SOLIDSYSLOGLWIPRAWTCPSTREAM_H

#include "ExternC.h"
#include "SolidSyslogSleep.h"
#include "SolidSyslogTcpConnectTimeoutFunction.h"

EXTERN_C_BEGIN

    struct SolidSyslogStream;

    struct SolidSyslogLwipRawTcpStreamConfig
    {
        SolidSyslogTcpConnectTimeoutFunction
            GetConnectTimeoutMs; /* NULL → use SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS tunable */
        void* ConnectTimeoutContext; /* passed through to GetConnectTimeoutMs; NULL is fine */
        SolidSyslogSleepFunction
            Sleep; /* required — drives the bounded-connect spin; NULL config falls back to NullStream */
    };

    struct SolidSyslogStream* SolidSyslogLwipRawTcpStream_Create(const struct SolidSyslogLwipRawTcpStreamConfig* config
    );
    void SolidSyslogLwipRawTcpStream_Destroy(struct SolidSyslogStream * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGLWIPRAWTCPSTREAM_H */
