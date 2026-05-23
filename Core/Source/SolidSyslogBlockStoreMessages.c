#include <stdint.h>

#include "SolidSyslogBlockStoreErrors.h"
#include "SolidSyslogError.h"

static const char* BlockStoreError_AsString(uint8_t code)
{
    static const char* const messages[BLOCKSTORE_ERROR_MAX] = {
        [BLOCKSTORE_ERROR_POOL_EXHAUSTED] = "SolidSyslogBlockStore_Create pool exhausted; returning fallback store",
        [BLOCKSTORE_ERROR_UNKNOWN_DESTROY] =
            "SolidSyslogBlockStore_Destroy called with a handle not issued by this pool",
    };
    const char* result = "unknown";
    if (code < (uint8_t) BLOCKSTORE_ERROR_MAX)
    {
        enum SolidSyslogBlockStoreErrors typed = (enum SolidSyslogBlockStoreErrors) code;
        result = messages[typed];
    }
    return result;
}

const struct SolidSyslogErrorSource BlockStoreErrorSource = {"BlockStore", BlockStoreError_AsString};
