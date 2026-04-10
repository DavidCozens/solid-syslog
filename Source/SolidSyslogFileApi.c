#include "SolidSyslogFileApiDefinition.h"

bool SolidSyslogFileApi_Open(struct SolidSyslogFileApi* api, const char* path)
{
    return api->Open(api, path);
}

void SolidSyslogFileApi_Close(struct SolidSyslogFileApi* api)
{
    api->Close(api);
}

bool SolidSyslogFileApi_IsOpen(struct SolidSyslogFileApi* api)
{
    return api->IsOpen(api);
}

bool SolidSyslogFileApi_Read(struct SolidSyslogFileApi* api, void* buf, size_t count)
{
    return api->Read(api, buf, count);
}

bool SolidSyslogFileApi_Write(struct SolidSyslogFileApi* api, const void* buf, size_t count)
{
    return api->Write(api, buf, count);
}

void SolidSyslogFileApi_SeekTo(struct SolidSyslogFileApi* api, size_t offset)
{
    api->SeekTo(api, offset);
}

size_t SolidSyslogFileApi_Size(struct SolidSyslogFileApi* api)
{
    return api->Size(api);
}

void SolidSyslogFileApi_Truncate(struct SolidSyslogFileApi* api)
{
    api->Truncate(api);
}
