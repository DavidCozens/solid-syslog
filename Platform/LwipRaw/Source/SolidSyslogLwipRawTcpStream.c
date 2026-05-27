#include "SolidSyslogLwipRawTcpStreamPrivate.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/arch.h"
#include "lwip/err.h"
#include "lwip/ip.h"
#include "lwip/ip_addr.h"
#include "lwip/tcp.h"
#include "SolidSyslogLwipRawAddressPrivate.h"
#include "SolidSyslogNullStream.h"
#include "SolidSyslogStream.h"
#include "SolidSyslogStreamDefinition.h"
#include "SolidSyslogTunables.h"

struct SolidSyslogAddress;

static uint32_t LwipRawTcpStream_NullConnectTimeoutGetter(void* context);

static bool LwipRawTcpStream_Open(struct SolidSyslogStream* base, const struct SolidSyslogAddress* addr);
static void LwipRawTcpStream_Close(struct SolidSyslogStream* base);

static inline struct SolidSyslogLwipRawTcpStream* LwipRawTcpStream_SelfFromBase(struct SolidSyslogStream* base);
static inline bool LwipRawTcpStream_ConfigProvidesGetter(const struct SolidSyslogLwipRawTcpStreamConfig* config);
static inline bool LwipRawTcpStream_IsOpen(const struct SolidSyslogLwipRawTcpStream* self);
static struct tcp_pcb* LwipRawTcpStream_OpenAndConfigurePcb(struct SolidSyslogLwipRawTcpStream* self);
static bool LwipRawTcpStream_ConnectOrAbortOnFailure(
    struct SolidSyslogLwipRawTcpStream* self,
    const struct SolidSyslogAddress* addr
);
static bool LwipRawTcpStream_TryConnect(
    struct SolidSyslogLwipRawTcpStream* self,
    const struct SolidSyslogAddress* addr
);
static bool LwipRawTcpStream_WaitForConnectedCallback(struct SolidSyslogLwipRawTcpStream* self);
static uint32_t LwipRawTcpStream_ResolveConnectTimeoutMs(struct SolidSyslogLwipRawTcpStream* self);
static void LwipRawTcpStream_AbortAndForgetPcb(struct SolidSyslogLwipRawTcpStream* self);
static void LwipRawTcpStream_ClosePcb(struct SolidSyslogLwipRawTcpStream* self);

static err_t LwipRawTcpStream_ConnectedCallback(void* arg, struct tcp_pcb* pcb, err_t err);
static err_t LwipRawTcpStream_RecvCallback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err);
static err_t LwipRawTcpStream_SentCallback(void* arg, struct tcp_pcb* tpcb, u16_t len);
static void LwipRawTcpStream_ErrCallback(void* arg, err_t err);

void LwipRawTcpStream_Initialise(
    struct SolidSyslogStream* base,
    const struct SolidSyslogLwipRawTcpStreamConfig* config
)
{
    static const struct SolidSyslogLwipRawTcpStream DefaultLwipRawTcpStream = {
        .Base = {.Open = LwipRawTcpStream_Open, .Send = NULL, .Read = NULL, .Close = LwipRawTcpStream_Close},
        .Config = {.GetConnectTimeoutMs = LwipRawTcpStream_NullConnectTimeoutGetter,
                   .ConnectTimeoutContext = NULL,
                   .Sleep = NULL},
        .Pcb = NULL,
        .Connected = false,
        .Errored = false,
        .RxQueue = {0},
        .RxQueueHead = 0,
        .RxQueueCount = 0,
        .RxHeadOffset = 0,
    };

    struct SolidSyslogLwipRawTcpStream* self = LwipRawTcpStream_SelfFromBase(base);
    *self = DefaultLwipRawTcpStream;
    self->Config.Sleep = config->Sleep;
    self->Config.ConnectTimeoutContext = config->ConnectTimeoutContext;
    if (LwipRawTcpStream_ConfigProvidesGetter(config))
    {
        self->Config.GetConnectTimeoutMs = config->GetConnectTimeoutMs;
    }
}

static inline bool LwipRawTcpStream_ConfigProvidesGetter(const struct SolidSyslogLwipRawTcpStreamConfig* config)
{
    return config->GetConnectTimeoutMs != NULL;
}

/* Null Object substituted when the integrator does not install a getter —
 * returns the compile-time tunable so the bounded-wait path has a single
 * code path regardless of whether the integrator wired runtime tuning. */
static uint32_t LwipRawTcpStream_NullConnectTimeoutGetter(void* context)
{
    (void) context;
    return (uint32_t) SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS;
}

static inline struct SolidSyslogLwipRawTcpStream* LwipRawTcpStream_SelfFromBase(struct SolidSyslogStream* base)
{
    return (struct SolidSyslogLwipRawTcpStream*) base;
}

void LwipRawTcpStream_Cleanup(struct SolidSyslogStream* base)
{
    LwipRawTcpStream_Close(base);
    /* Overwrite the abstract base with the shared NullStream vtable so
     * use-after-destroy is a safe no-op rather than a NULL-fn-pointer crash. */
    *base = *SolidSyslogNullStream_Get();
}

static bool LwipRawTcpStream_Open(struct SolidSyslogStream* base, const struct SolidSyslogAddress* addr)
{
    struct SolidSyslogLwipRawTcpStream* self = LwipRawTcpStream_SelfFromBase(base);
    if (!LwipRawTcpStream_IsOpen(self))
    {
        self->Pcb = LwipRawTcpStream_OpenAndConfigurePcb(self);
        if (LwipRawTcpStream_IsOpen(self))
        {
            (void) LwipRawTcpStream_ConnectOrAbortOnFailure(self, addr);
        }
    }
    return LwipRawTcpStream_IsOpen(self);
}

static inline bool LwipRawTcpStream_IsOpen(const struct SolidSyslogLwipRawTcpStream* self)
{
    return self->Pcb != NULL;
}

static struct tcp_pcb* LwipRawTcpStream_OpenAndConfigurePcb(struct SolidSyslogLwipRawTcpStream* self)
{
    struct tcp_pcb* pcb = tcp_new();
    if (pcb != NULL)
    {
        ip_set_option(pcb, SOF_KEEPALIVE);
        tcp_arg(pcb, self);
        tcp_recv(pcb, LwipRawTcpStream_RecvCallback);
        tcp_sent(pcb, LwipRawTcpStream_SentCallback);
        tcp_err(pcb, LwipRawTcpStream_ErrCallback);
    }
    return pcb;
}

static bool LwipRawTcpStream_ConnectOrAbortOnFailure(
    struct SolidSyslogLwipRawTcpStream* self,
    const struct SolidSyslogAddress* addr
)
{
    bool connected = LwipRawTcpStream_TryConnect(self, addr);
    if (!connected)
    {
        LwipRawTcpStream_AbortAndForgetPcb(self);
    }
    return connected;
}

static bool LwipRawTcpStream_TryConnect(
    struct SolidSyslogLwipRawTcpStream* self,
    const struct SolidSyslogAddress* addr
)
{
    const struct SolidSyslogLwipRawAddress* dst = SolidSyslogLwipRawAddress_AsConst(addr);
    self->Connected = false;
    self->Errored = false;
    err_t connectErr = tcp_connect(self->Pcb, &dst->Ip, dst->Port, LwipRawTcpStream_ConnectedCallback);
    bool ok = false;
    if (connectErr == ERR_OK)
    {
        ok = LwipRawTcpStream_WaitForConnectedCallback(self);
    }
    return ok;
}

/* Bounded synchronous-Open spin: each iteration sleeps via the
 * integrator-injected Sleep so lwIP's timer / RX paths get cycles to
 * advance the SYN/SYN-ACK exchange. Exits on Connected (success),
 * Errored (set by connected_cb on non-ERR_OK or by tcp_err), or
 * elapsed >= deadline (timeout). */
static bool LwipRawTcpStream_WaitForConnectedCallback(struct SolidSyslogLwipRawTcpStream* self)
{
    const uint32_t pollMs = (uint32_t) SOLIDSYSLOG_LWIP_RAW_TCP_CONNECT_POLL_MS;
    const uint32_t deadlineMs = LwipRawTcpStream_ResolveConnectTimeoutMs(self);
    uint32_t elapsedMs = 0;
    while (!self->Connected && !self->Errored && (elapsedMs < deadlineMs))
    {
        self->Config.Sleep((int) pollMs);
        elapsedMs += pollMs;
    }
    return self->Connected;
}

/* Bridges the integrator-installed getter (or the Null Object substituted
 * in Initialise) to the bounded spin deadline. Invoked on every connect
 * attempt so a runtime-tunable value takes effect on the next reconnect. */
static uint32_t LwipRawTcpStream_ResolveConnectTimeoutMs(struct SolidSyslogLwipRawTcpStream* self)
{
    return self->Config.GetConnectTimeoutMs(self->Config.ConnectTimeoutContext);
}

static void LwipRawTcpStream_AbortAndForgetPcb(struct SolidSyslogLwipRawTcpStream* self)
{
    tcp_abort(self->Pcb);
    self->Pcb = NULL;
}

static void LwipRawTcpStream_Close(struct SolidSyslogStream* base)
{
    struct SolidSyslogLwipRawTcpStream* self = LwipRawTcpStream_SelfFromBase(base);
    LwipRawTcpStream_ClosePcb(self);
}

/* tcp_close must NOT be called on a pcb that has already been released by
 * tcp_err — that's a use-after-free in lwIP. The Pcb != NULL guard works
 * because LwipRawTcpStream_ErrCallback nulls Pcb when lwIP releases the
 * pcb on its side. */
static void LwipRawTcpStream_ClosePcb(struct SolidSyslogLwipRawTcpStream* self)
{
    if (LwipRawTcpStream_IsOpen(self))
    {
        (void) tcp_close(self->Pcb);
        self->Pcb = NULL;
    }
}

static err_t LwipRawTcpStream_ConnectedCallback(void* arg, struct tcp_pcb* pcb, err_t err)
{
    (void) pcb;
    struct SolidSyslogLwipRawTcpStream* self = (struct SolidSyslogLwipRawTcpStream*) arg;
    if (err == ERR_OK)
    {
        self->Connected = true;
    }
    else
    {
        self->Errored = true;
    }
    return ERR_OK;
}

/* RX queue + tcp_recved drain (and pbuf_free of the head pbuf when drained)
 * land in the Send/Read slice. For now the callback is a no-op stub — lwIP
 * requires the slot wired before tcp_connect so the recv path is set up by
 * the time the peer sends data, even though the wrapper's Stream_Read does
 * not yet drain. */
static err_t LwipRawTcpStream_RecvCallback(void* arg, struct tcp_pcb* tpcb, struct pbuf* p, err_t err)
{
    (void) arg;
    (void) tpcb;
    (void) p;
    (void) err;
    return ERR_OK;
}

/* Real tcp_sent handling is unused under TCP_WRITE_FLAG_COPY (Commit 3
 * decision 1) — caller buffers are released at Send return, not at
 * peer-ACK time. The callback exists because lwIP requires the slot set. */
static err_t LwipRawTcpStream_SentCallback(void* arg, struct tcp_pcb* tpcb, u16_t len)
{
    (void) arg;
    (void) tpcb;
    (void) len;
    return ERR_OK;
}

/* lwIP fires tcp_err for fatal events (RST, OOM, ABRT) AFTER releasing the
 * pcb upstream — we must null our Pcb pointer and NOT call tcp_close.
 * Subsequent Stream_Close sees Pcb == NULL and is a safe no-op. */
static void LwipRawTcpStream_ErrCallback(void* arg, err_t err)
{
    (void) err;
    struct SolidSyslogLwipRawTcpStream* self = (struct SolidSyslogLwipRawTcpStream*) arg;
    self->Pcb = NULL;
    self->Errored = true;
}
