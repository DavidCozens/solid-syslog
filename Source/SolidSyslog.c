#include "SolidSyslog.h"
#include "SolidSyslogSender.h"

struct SolidSyslog
{
    struct SolidSyslogSender* sender;
    SolidSyslogFreeFn         free;
};

struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslogConfig* config)
{
    struct SolidSyslog* instance = config->alloc(sizeof(struct SolidSyslog));
    if (instance != NULL)
    {
        instance->sender = config->sender;
        instance->free   = config->free;
    }
    return instance;
}

void SolidSyslog_Destroy(struct SolidSyslog* logger)
{
    logger->free(logger);
}

void SolidSyslog_Log(struct SolidSyslog* logger)
{
    static const char message[] = "<134>1 2009-03-23T00:00:00.000Z TestHost TestApp 42 54 - hello world";
    SolidSyslogSender_Send(logger->sender, message, sizeof(message) - 1);
}
