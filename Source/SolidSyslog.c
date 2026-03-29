#include "SolidSyslog.h"
#include "SolidSyslog_Sender.h"

struct SolidSyslog
{
    struct SolidSyslog_Sender *sender;
};

static struct SolidSyslog instance;

struct SolidSyslog *SolidSyslog_Create(const struct SolidSyslog_Config *config)
{
    instance.sender = config->sender;
    return &instance;
}

void SolidSyslog_Destroy(struct SolidSyslog *logger)
{
    (void)logger;
}

void SolidSyslog_Log(struct SolidSyslog *logger)
{
    logger->sender->Send(logger->sender, NULL, 0);
}
