#ifndef SOLIDSYSLOGFILEBLOCKDEVICE_H
#define SOLIDSYSLOGFILEBLOCKDEVICE_H

#include <stddef.h>

#include "ExternC.h"

struct SolidSyslogBlockDevice;
struct SolidSyslogFile;

EXTERN_C_BEGIN

    /** Back a BlockDevice with one file per block, named @p pathPrefix followed by
     *  a two-digit block index and ".log" (so block 0 is "<prefix>00.log"; indices
     *  above 99 are rejected). The driver caches at most one open @p file handle,
     *  re-pointing it only when the target block changes, which enforces the
     *  single-open-handle-per-path invariant the storage layer relies on. @p file
     *  must outlive the device. @p blockSize is the per-block capacity reported via
     *  SolidSyslogBlockDevice_GetBlockSize; pass SOLIDSYSLOG_FILE_DEFAULT_BLOCK_SIZE
     *  (or 0) for the default. An exhausted pool falls back to the shared
     *  NullBlockDevice. */
    struct SolidSyslogBlockDevice* SolidSyslogFileBlockDevice_Create(
        struct SolidSyslogFile * file,
        const char* pathPrefix,
        size_t blockSize
    );
    /** Close the cached handle and release the pool slot; does not destroy the
     *  injected file. */
    void SolidSyslogFileBlockDevice_Destroy(struct SolidSyslogBlockDevice * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGFILEBLOCKDEVICE_H */
