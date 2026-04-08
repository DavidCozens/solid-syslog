#include "SolidSyslogStoreDefinition.h"

bool SolidSyslogStore_Write(struct SolidSyslogStore* store, const void* data, size_t size)
{
    return store->Write(store, data, size);
}

bool SolidSyslogStore_ReadNextUnsent(struct SolidSyslogStore* store, void* data, size_t maxSize, size_t* bytesRead)
{
    return store->ReadNextUnsent(store, data, maxSize, bytesRead);
}

void SolidSyslogStore_MarkSent(struct SolidSyslogStore* store)
{
    store->MarkSent(store);
}

bool SolidSyslogStore_HasUnsent(struct SolidSyslogStore* store)
{
    return store->HasUnsent(store);
}
