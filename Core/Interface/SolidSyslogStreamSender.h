#ifndef SOLIDSYSLOG_STREAM_SENDER_H
#define SOLIDSYSLOG_STREAM_SENDER_H

#include "SolidSyslogEndpoint.h"
#include "ExternC.h"

EXTERN_C_BEGIN

    struct SolidSyslogSender;

    /** Wiring for SolidSyslogStreamSender_Create, a sender that octet-frames each
     *  message (RFC 6587) over any injected Stream (plain TCP, TLS, or a
     *  caller-supplied byte transport). Resolver, Stream, and Address are required
     *  (NULL makes Create fall back to the shared NullSender); the rest is
     *  optional. Everything you inject must outlive the created handle, but the
     *  config struct itself is copied at Create and may be transient. */
    struct SolidSyslogStreamSenderConfig
    {
        struct SolidSyslogResolver* Resolver;
        struct SolidSyslogStream* Stream;
        /** Destination slot the Resolver writes and the Stream opens against; one per sender. */
        struct SolidSyslogAddress* Address;
        /** Fills host/port; called only on (re)connect. NULL resolves an empty host on port 0. */
        SolidSyslogEndpointFunction Endpoint;
        /** Polled every Send to detect an endpoint change; NULL pins the destination (never reconnects for a change). */
        SolidSyslogEndpointVersionFunction EndpointVersion;
    };

    /** Create a stream sender from @p config. Never returns NULL: a NULL or
     *  invalid config, or an exhausted sender pool, reports via SolidSyslog_Error
     *  and returns the shared NullSender (Send drops on the floor), so the result
     *  is safe to wire without a null-check. */
    struct SolidSyslogSender* SolidSyslogStreamSender_Create(const struct SolidSyslogStreamSenderConfig* config);

    /** Release the sender's pool slot, closing the stream first. Does not free the
     *  injected Resolver, Stream, or Address; the caller owns those. */
    void SolidSyslogStreamSender_Destroy(struct SolidSyslogSender * base);

EXTERN_C_END

#endif /* SOLIDSYSLOG_STREAM_SENDER_H */
