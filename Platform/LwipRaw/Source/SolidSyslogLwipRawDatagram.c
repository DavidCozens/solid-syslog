#include "SolidSyslogLwipRawDatagramPrivate.h"

#include <stdbool.h>
#include <stddef.h>

#include "lwip/udp.h"
#include "SolidSyslogDatagramDefinition.h"
#include "SolidSyslogNullDatagram.h"

static bool LwipRawDatagram_Open(struct SolidSyslogDatagram* base);
static void LwipRawDatagram_Close(struct SolidSyslogDatagram* base);

static inline struct SolidSyslogLwipRawDatagram* LwipRawDatagram_SelfFromBase(struct SolidSyslogDatagram* base);
static inline bool LwipRawDatagram_IsOpen(const struct SolidSyslogLwipRawDatagram* self);

void LwipRawDatagram_Initialise(struct SolidSyslogDatagram* base)
{
    struct SolidSyslogLwipRawDatagram* self = LwipRawDatagram_SelfFromBase(base);
    self->Base.Open = LwipRawDatagram_Open;
    self->Base.Close = LwipRawDatagram_Close;
    self->Pcb = NULL;
}

static inline struct SolidSyslogLwipRawDatagram* LwipRawDatagram_SelfFromBase(struct SolidSyslogDatagram* base)
{
    return (struct SolidSyslogLwipRawDatagram*) base;
}

void LwipRawDatagram_Cleanup(struct SolidSyslogDatagram* base)
{
    LwipRawDatagram_Close(base);
    /* Overwrite the abstract base with the shared NullDatagram vtable so
     * use-after-destroy is a safe no-op rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullDatagram_Get();
}

static void LwipRawDatagram_Close(struct SolidSyslogDatagram* base)
{
    struct SolidSyslogLwipRawDatagram* self = LwipRawDatagram_SelfFromBase(base);
    if (LwipRawDatagram_IsOpen(self))
    {
        udp_remove(self->Pcb);
        self->Pcb = NULL;
    }
}

static bool LwipRawDatagram_Open(struct SolidSyslogDatagram* base)
{
    struct SolidSyslogLwipRawDatagram* self = LwipRawDatagram_SelfFromBase(base);
    if (!LwipRawDatagram_IsOpen(self))
    {
        self->Pcb = udp_new();
    }
    return LwipRawDatagram_IsOpen(self);
}

static inline bool LwipRawDatagram_IsOpen(const struct SolidSyslogLwipRawDatagram* self)
{
    return self->Pcb != NULL;
}
