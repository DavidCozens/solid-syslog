#ifndef SOLIDSYSLOG_H
#define SOLIDSYSLOG_H

#include "SolidSyslog_Alloc.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct SolidSyslog_Sender;
    struct SolidSyslog;

    struct SolidSyslog_Config
    {
        struct SolidSyslog_Sender* sender;
        SolidSyslog_AllocFn        alloc;
        SolidSyslog_FreeFn         free;
    };

    struct SolidSyslog* SolidSyslog_Create(const struct SolidSyslog_Config* config);
    void                SolidSyslog_Destroy(struct SolidSyslog* logger);
    void                SolidSyslog_Log(struct SolidSyslog* logger);

#ifdef __cplusplus
}
#endif

#endif /* SOLIDSYSLOG_H */
