#ifndef SOLIDSYSLOG_TCP_SENDER_H
#define SOLIDSYSLOG_TCP_SENDER_H

#include "ExternC.h"

struct SolidSyslogSender;

enum
{
    SOLIDSYSLOG_TCP_DEFAULT_PORT = 6514
};

struct SolidSyslogTcpSenderConfig
{
    int (*getPort)(void);         // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
    const char* (*getHost)(void); // NOLINT(modernize-redundant-void-arg) -- C idiom; void is required in C to mean "no parameters"
};

EXTERN_C_BEGIN

    struct SolidSyslogSender* SolidSyslogTcpSender_Create(const struct SolidSyslogTcpSenderConfig* config);
    void                      SolidSyslogTcpSender_Destroy(void);

EXTERN_C_END

#endif /* SOLIDSYSLOG_TCP_SENDER_H */
