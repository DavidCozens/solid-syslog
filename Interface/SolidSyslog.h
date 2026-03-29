#ifndef SOLIDSYSLOG_H
#define SOLIDSYSLOG_H

#ifdef __cplusplus
extern "C"
{
#endif

    struct SolidSyslog_Sender;
    struct SolidSyslog;

    struct SolidSyslog_Config
    {
        struct SolidSyslog_Sender *sender;
    };

    struct SolidSyslog *SolidSyslog_Create(const struct SolidSyslog_Config *config);
    void SolidSyslog_Destroy(struct SolidSyslog *logger);
    void SolidSyslog_Log(struct SolidSyslog *logger);

#ifdef __cplusplus
}
#endif

#endif /* SOLIDSYSLOG_H */
