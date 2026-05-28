# Integrating SolidSyslog with lwIP (Raw API)

`Platform/LwipRaw/` wraps lwIP's Raw API to provide the same
`SolidSyslogDatagram` / `SolidSyslogStream` / `SolidSyslogAddress` /
`SolidSyslogResolver` vtables the rest of the library composes against.
It is the right choice when:

- Your target runs lwIP — bare-metal, FreeRTOS, Zephyr, ThreadX, NuttX
  — including `NO_SYS=1` deployments where sockets/NETCONN aren't
  available.
- You want to share TCP/IP between SolidSyslog and other lwIP-using
  subsystems (HTTP server, MQTT client, OTA updater) without
  duplicating the stack.
- You're on FreeRTOS but prefer lwIP to FreeRTOS-Plus-TCP for licence
  or sizing reasons. Both backends ship in the same library; pick at
  CMake time with `SOLIDSYSLOG_FREERTOS_NET=LWIP`.

This document covers what *you*, the integrator, plug in. It does not
re-teach lwIP — for that, see the
[upstream lwIP documentation](https://www.nongnu.org/lwip/2_1_x/index.html).

---

## What ships in `Platform/LwipRaw/`

| Class | Wraps | Purpose |
|---|---|---|
| `SolidSyslogLwipRawAddress` | `ip_addr_t` + `u16_t` port | Destination handle the Resolver writes into and the Datagram/TcpStream read from. |
| `SolidSyslogLwipRawResolver` | `ipaddr_aton` | Synchronous numeric IPv4 parsing. Rejects DNS names — they need the future `SolidSyslogLwipRawDnsResolver` (S28.07). |
| `SolidSyslogLwipRawDatagram` | `udp_new` / `udp_sendto` / `udp_remove` | UDP sender. Zero-copy `PBUF_REF` send. |
| `SolidSyslogLwipRawTcpStream` | `tcp_new` / `tcp_connect` / `tcp_write` / `tcp_output` / `tcp_recv` / `tcp_recved` / `tcp_close` / `tcp_abort` | TCP byte transport. Bounded synchronous Open. Bounded RX pbuf queue. |

`Platform/LwipRaw/Source/` is **OS-agnostic** — it wraps lwIP only and
contains zero direct calls to FreeRTOS, POSIX, Win32, Zephyr, or any
other host primitive. The one host primitive the TcpStream needs (a
bounded sleep for the synchronous-Open spin loop) is abstracted behind
the `SolidSyslogSleepFunction` typedef and supplied by you at
configure time.

mbedTLS layering is unchanged — `SolidSyslogMbedTlsStream` consumes
`SolidSyslogLwipRawTcpStream` as its byte transport without
modification. See [`docs/integrating-mbedtls.md`](integrating-mbedtls.md)
for the TLS side.

---

## `NO_SYS=1` vs `NO_SYS=0`

lwIP supports both threading models. SolidSyslog supports both — the
adapter code is the same; the difference is entirely in how *you*
drive lwIP forward.

### `NO_SYS=1` (bare-metal main-loop)

Your `main()` is a forever-loop that, on each pass, calls
`sys_check_timeouts()` and drives the RX path
(`netif->input()` / `ethernetif_input()` / whichever your BSP wires).
Every lwIP Raw API call must happen on that same thread.

SolidSyslog's `Service` loop fits this naturally — call it from your
main loop alongside `sys_check_timeouts()`. The TcpStream's bounded
synchronous-Open spin loop calls your injected `Sleep` callback
between polls; under `NO_SYS=1` your Sleep implementation should
**tick the lwIP machinery** while it waits:

```c
void MyLwipSleep(int milliseconds)
{
    uint32_t deadline = MyTimebase_NowMs() + (uint32_t) milliseconds;
    while (MyTimebase_NowMs() < deadline)
    {
        sys_check_timeouts();
        MyNetif_DrivePolledRx(); /* your BSP's RX pump */
    }
}
```

Without this, `tcp_connect`'s `connected_cb` never fires (lwIP can't
advance its state machine while you sleep), and Open times out.

### `NO_SYS=0` (tcpip thread)

lwIP runs a dedicated `tcpip` thread that owns its state machine; you
post work to it via `tcpip_callback()` or use the BSD Sockets /
NETCONN APIs.

**Threading rule for the Raw API**: every Raw API call from outside
the tcpip thread must be marshalled via `tcpip_callback()` (or via
the `LWIP_TCPIP_CORE_LOCKING_INPUT` lock if you've compiled with core
locking). This applies to every call SolidSyslog's adapters make into
lwIP — Datagram's `udp_sendto`, TcpStream's `tcp_write`, etc.

**SolidSyslog does not marshal internally.** That would force every
integrator to compile `tcpip.c` even when they're on `NO_SYS=1`. The
expectation is that *you* call SolidSyslog APIs on the tcpip thread,
either by running your Service loop on it, or by marshalling each
call at the SolidSyslog API boundary:

```c
static void DoService(void* ctx)
{
    SolidSyslog_Service((struct SolidSyslog*) ctx);
}

void MyServiceTick(struct SolidSyslog* handle)
{
    (void) tcpip_callback(DoService, handle);
}
```

Your injected `Sleep` callback under `NO_SYS=0` is just a yield —
typically `vTaskDelay(pdMS_TO_TICKS(milliseconds))` on FreeRTOS:

```c
void MyLwipSleep(int milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS((uint32_t) milliseconds));
}
```

The tcpip thread runs concurrently and processes the SYN/SYN-ACK
exchange while you yield.

---

## `lwipopts.h` expectations

The adapter wraps a specific subset of lwIP — your `lwipopts.h` needs
those features compiled in. Defaults that already cover us are noted;
features you must enable are flagged.

| Setting | Required | Notes |
|---|---|---|
| `LWIP_RAW=1` | **Yes** | The whole point — Raw API. |
| `LWIP_UDP=1` | **Yes (Datagram)** | Wraps `udp_*`. |
| `LWIP_TCP=1` | **Yes (TcpStream)** | Wraps `tcp_*`. |
| `LWIP_DNS` | No | The current Resolver only parses numeric IPv4 via `ipaddr_aton`. DNS lands in S28.07. |
| `ARP_QUEUEING=1` | **Recommended** | lwIP default. With it, the first datagram to an unresolved peer is `pbuf_clone`d into PBUF_RAM and queued behind the ARP request — when the reply lands, the packet ships. With `ARP_QUEUEING=0` the first datagram is silently dropped at the IP layer; cold-start logging loses messages. |
| `LWIP_TCP_KEEPALIVE=1` | **Recommended** | Without this, the `SOF_KEEPALIVE` bit the adapter sets on every pcb is a no-op. Tune `TCP_KEEPIDLE_DEFAULT` / `TCP_KEEPINTVL_DEFAULT` / `TCP_KEEPCNT_DEFAULT` for your deadline budget. |
| `TCP_MSS` | Per-platform | Default `536` (RFC-conservative). Bump to `1460` on Ethernet links if your MTU is 1500 and you want fewer segments per syslog record. |
| `PBUF_POOL_SIZE` | Per-traffic | Size the pool generously. The Datagram path borrows one `MEMP_PBUF` header per send; the TcpStream RX path borrows one pbuf per segment until `Stream_Read` drains it. |
| `MEMP_NUM_TCP_PCB` | Per-deployment | At least the number of concurrent `SolidSyslogLwipRawTcpStream` instances (default pool = 2 for the TLS-over-plain-TCP pair). |
| `MEMP_NUM_UDP_PCB` | Per-deployment | At least the number of concurrent `SolidSyslogLwipRawDatagram` instances (default pool = 1). |

---

## Wiring example — bare-metal `NO_SYS=1`

```c
#include "SolidSyslog.h"
#include "SolidSyslogConfig.h"
#include "SolidSyslogLwipRawAddress.h"
#include "SolidSyslogLwipRawDatagram.h"
#include "SolidSyslogLwipRawResolver.h"
#include "SolidSyslogLwipRawTcpStream.h"
#include "SolidSyslogPassthroughBuffer.h"
#include "SolidSyslogStreamSender.h"
#include "SolidSyslogUdpSender.h"

extern void MyLwipSleep(int milliseconds);  /* sys_check_timeouts + RX pump */

static struct SolidSyslog*       g_syslog;
static struct SolidSyslogBuffer* g_buffer;

void LogPipelineInit(void)
{
    struct SolidSyslogResolver*  resolver  = SolidSyslogLwipRawResolver_Create();
    struct SolidSyslogAddress*   udpAddr   = SolidSyslogLwipRawAddress_Create();
    struct SolidSyslogDatagram*  datagram  = SolidSyslogLwipRawDatagram_Create();

    struct SolidSyslogUdpSenderConfig udpCfg = {
        .Resolver         = resolver,
        .Datagram         = datagram,
        .Address          = udpAddr,
        .Endpoint         = MyEndpoint,         /* your SolidSyslogEndpointFunction */
        .EndpointVersion  = MyEndpointVersion,
    };
    struct SolidSyslogSender* sender = SolidSyslogUdpSender_Create(&udpCfg);

    g_buffer = SolidSyslogPassthroughBuffer_Create(sender);

    struct SolidSyslogConfig syslogCfg = {
        .Hostname  = MyHostname,
        .AppName   = MyAppName,
        .ProcessId = MyProcessId,
        .Clock     = MyClock,
        .Buffer    = g_buffer,
    };
    g_syslog = SolidSyslog_Create(&syslogCfg);
}

void MainLoop(void)
{
    for (;;)
    {
        sys_check_timeouts();
        MyNetif_DrivePolledRx();
        SolidSyslog_Service(g_syslog);
        /* … rest of your application … */
    }
}
```

For TCP, swap the UDP sender for a `SolidSyslogStreamSender` whose
`Stream` is a `SolidSyslogLwipRawTcpStream` built with your `Sleep`:

```c
struct SolidSyslogLwipRawTcpStreamConfig streamCfg = {
    .GetConnectTimeoutMs   = NULL,        /* falls back to SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS */
    .ConnectTimeoutContext = NULL,
    .Sleep                 = MyLwipSleep, /* required */
};
struct SolidSyslogStream* tcpStream = SolidSyslogLwipRawTcpStream_Create(&streamCfg);
```

---

## Adapter-specific notes

### Datagram — pbuf strategy

`SolidSyslogLwipRawDatagram` uses `PBUF_REF`: a single pbuf header is
allocated per `SendTo`, its `payload` is pointed at the caller's
buffer, `udp_sendto` is called, and the header is `pbuf_free`d before
return. Zero copy on the hot path.

This is safe across ARP queueing because lwIP's `etharp_query` does
`pbuf_clone(…, PBUF_RAM, q)` — it copies the referenced payload into
a private RAM pbuf before queueing, so the caller's buffer only needs
to live for the `udp_sendto` call itself (which is the synchronous
guarantee `SolidSyslogDatagram_SendTo` already provides).

### TcpStream — `tcp_write` strategy

`SolidSyslogLwipRawTcpStream` uses `TCP_WRITE_FLAG_COPY`: lwIP copies
your bytes into its own pbufs before `tcp_write` returns. This costs
one `memcpy` per send but honours the synchronous `Stream_Send(buf,
len)` lifetime contract — caller buffers are free at return,
regardless of when the peer ACKs.

`tcp_output` is called after every successful `tcp_write` to nudge
transmission. If `tcp_output` returns `ERR_MEM`, the data is already
in `pcb->snd_buf` — lwIP will retry on the next `tcp_tmr` tick and
the wrapper reports Send-success (lwIP owns the bytes, exactly
matching POSIX's "kernel accepted the data into the send buffer"
semantics).

### TcpStream — synchronous Open via spin-with-sleep

`tcp_connect` is asynchronous: it returns immediately and lwIP fires
the registered `connected_cb` when the SYN/SYN-ACK exchange
completes. `SolidSyslogStream_Open` is synchronous. The wrapper
bridges by spinning on a `Connected` flag set by its `connected_cb`,
sleeping `SOLIDSYSLOG_LWIP_RAW_TCP_CONNECT_POLL_MS` (default 10 ms)
between checks via your injected `Sleep`, bounded by the
`GetConnectTimeoutMs` getter (default `SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS`
= 200 ms — install a runtime getter per the S12.17 pattern if you
need to vary it).

Timeout → `tcp_abort` on the pcb, Open returns `false`. Errored
callback → `tcp_abort`, Open returns `false`. Immediate non-`ERR_OK`
from `tcp_connect` → `tcp_abort`, Open returns `false`.

### TcpStream — RX queue

lwIP's `tcp_recv` callback fires when bytes arrive. The wrapper owns
a bounded ring of pbuf pointers sized by `SOLIDSYSLOG_LWIP_RAW_TCP_RX_QUEUE_SIZE`
(default 8). Each `Stream_Read` drains bytes from the head pbuf,
calls `tcp_recved(pcb, n)` to ACK back to lwIP's receive window, and
`pbuf_free`s the head when fully drained. Queue full → the
callback returns non-`ERR_OK` so lwIP retains the pbuf and replays
the callback later (lwIP's flow-control hook).

The default-8 queue size is sized for the typical mTLS handshake
flight (ServerHello + Certificate + ServerKeyExchange +
ServerHelloDone is 2–4 segments). Bump it for streaming server
responses; lower it if your `MEMP_NUM_PBUF` is constrained and you
need lwIP to backpressure sooner.

### TcpStream — lifecycle ownership

The wrapper owns its `tcp_pcb` end-to-end. Three things to know:

1. **`tcp_err` releases the pcb upstream.** When lwIP fires
   `tcp_err` for a fatal event (RST, OOM, ABRT), the pcb is gone
   from lwIP's side *before* the callback runs. The wrapper's
   `tcp_err` handler nulls its internal `Pcb` field and sets an
   `Errored` flag. The next `Stream_Send` returns `false`; the next
   `Stream_Read` returns `-1`. Crucially, calling `tcp_close` on a
   pcb that was already released by `tcp_err` is a **use-after-free
   in lwIP** — the wrapper guards against this with a `Pcb != NULL`
   check before `tcp_close`. **You never see this rule** unless you
   bypass the abstraction and poke at lwIP pcbs directly through
   your own code — don't.

2. **Peer FIN (`tcp_recv` with `p == NULL`) drains before EOF.**
   The half-close sets `Errored`; the next `Stream_Read` that finds
   the queue empty returns `-1` and internally `tcp_close`s the
   pcb. Already-queued bytes drain first.

3. **`Close` is idempotent.** Second `Close` is a no-op. `Destroy`
   internally calls `Close` (which drains the RX queue's pbufs and
   then `tcp_close`s if the pcb is still around), then overwrites
   the abstract base with `SolidSyslogNullStream` so use-after-
   destroy is a safe no-op rather than a NULL-fn-pointer crash.

---

## Tunables

All tunables live in `Core/Interface/SolidSyslogTunablesDefaults.h`.
Override by `#define`ing them in a user-tunables header passed via
the `SOLIDSYSLOG_USER_TUNABLES_FILE` CMake variable.

| Tunable | Default | Adjust when |
|---|---|---|
| `SOLIDSYSLOG_LWIP_RAW_RESOLVER_POOL_SIZE` | `1U` | You need multiple concurrent resolver instances (rare). |
| `SOLIDSYSLOG_LWIP_RAW_DATAGRAM_POOL_SIZE` | `1U` | You wire more than one UDP sender. |
| `SOLIDSYSLOG_LWIP_RAW_TCP_STREAM_POOL_SIZE` | `2U` | You wire more than the canonical plain-TCP + TLS-underlying-TCP pair. |
| `SOLIDSYSLOG_ADDRESS_POOL_SIZE` | `3U` | Shared with PlusTcp / Posix / Winsock — bump if you need >3 concurrent destinations. |
| `SOLIDSYSLOG_TCP_CONNECT_TIMEOUT_MS` | `200U` | Default suits loopback / LAN. Raise for WAN deployments behind a high-RTT link; or install a runtime `GetConnectTimeoutMs` getter for per-instance tuning. |
| `SOLIDSYSLOG_LWIP_RAW_TCP_CONNECT_POLL_MS` | `10U` | Default gives 20 polls inside the 200 ms connect deadline. Lower it to notice a fast connect sooner; raise it to reduce spin overhead on a constrained MCU. |
| `SOLIDSYSLOG_LWIP_RAW_TCP_RX_QUEUE_SIZE` | `8U` | Sized for the typical mTLS handshake flight. Bump for streaming server responses; lower if `MEMP_NUM_PBUF` is tight. |

---

## What this guide does not cover

- **DNS** — currently out of scope. `SolidSyslogLwipRawResolver` only
  parses numeric IPv4. `SolidSyslogLwipRawDnsResolver` lands in
  S28.07.
- **IPv6** — the current Address / Resolver are IPv4-only.
- **Multi-`netif` routing** — neither Datagram nor TcpStream selects
  an output interface; lwIP's routing table decides.
- **Jumbo-frame MTU discovery** — `Datagram_MaxPayload` returns
  `SOLIDSYSLOG_UDP_IPV6_SAFE_PAYLOAD` (1232 bytes) unconditionally.
- **BDD coverage** — the FreeRTOS-on-lwIP BDD target arrives in
  S28.06.
