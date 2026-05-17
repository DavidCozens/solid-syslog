#ifndef SOLIDSYSLOGATOMICCOUNTERTESTHELPER_H
#define SOLIDSYSLOGATOMICCOUNTERTESTHELPER_H

#include "ExternC.h"

#include <stdint.h>

EXTERN_C_BEGIN

    struct SolidSyslogAtomicCounter;

    enum
    {
        TEST_ATOMIC_COUNTER_STORAGE_SIZE = 64
    };

    typedef struct
    {
        intptr_t Slots[(TEST_ATOMIC_COUNTER_STORAGE_SIZE + sizeof(intptr_t) - 1U) / sizeof(intptr_t)];
    } TestAtomicCounterStorage;

    struct SolidSyslogAtomicCounter* TestAtomicCounter_Create(TestAtomicCounterStorage * storage);
    void TestAtomicCounter_Init(struct SolidSyslogAtomicCounter * base, uint32_t value);
    uint32_t TestAtomicCounter_Increment(struct SolidSyslogAtomicCounter * base);
    void TestAtomicCounter_Destroy(struct SolidSyslogAtomicCounter * base);

EXTERN_C_END

#endif /* SOLIDSYSLOGATOMICCOUNTERTESTHELPER_H */
