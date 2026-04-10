#ifndef SOLIDSYSLOGFILEAPI_H
#define SOLIDSYSLOGFILEAPI_H

#include "ExternC.h"
#include <stdbool.h>
#include <stddef.h>

EXTERN_C_BEGIN

    struct SolidSyslogFileApi;

    bool   SolidSyslogFileApi_Open(struct SolidSyslogFileApi* api, const char* path);
    void   SolidSyslogFileApi_Close(struct SolidSyslogFileApi* api);
    bool   SolidSyslogFileApi_IsOpen(struct SolidSyslogFileApi* api);
    bool   SolidSyslogFileApi_Read(struct SolidSyslogFileApi* api, void* buf, size_t count);
    bool   SolidSyslogFileApi_Write(struct SolidSyslogFileApi* api, const void* buf, size_t count);
    void   SolidSyslogFileApi_SeekTo(struct SolidSyslogFileApi* api, size_t offset);
    size_t SolidSyslogFileApi_Size(struct SolidSyslogFileApi* api);
    void   SolidSyslogFileApi_Truncate(struct SolidSyslogFileApi* api);

EXTERN_C_END

#endif /* SOLIDSYSLOGFILEAPI_H */
