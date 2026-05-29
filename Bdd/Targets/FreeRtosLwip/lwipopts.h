/* lwIP options for the QEMU mps2-an385 FreeRTOS + lwIP BDD target.
 *
 * S28.09 flipped this from the S28.07 link-probe (NO_SYS=1, no netif) to a
 * worked NO_SYS=0 runtime: lwIP runs its own "tcpip" thread, a LAN9118 netif
 * (netif/EthernetIf.c) drives the wire, and the SolidSyslog LwipRaw adapters
 * reach the core through the tcpip_callback marshal (S28.06). We still only
 * use lwIP's Raw API (the adapters call udp_ / tcp_ functions directly via the
 * marshal),
 * so the sequential netconn / socket API stays OFF — the tcpip thread exists
 * for RX delivery (tcpip_input), timeouts, and marshalled callbacks only.
 *
 * Memory is lwIP-pool managed (no libc malloc) so the footprint is the static,
 * embedded-realistic shape an integrator ships — not the host-test shortcut
 * (MEM_LIBC_MALLOC) in Tests/Support/LwipFakes/Interface/lwipopts.h. */
#ifndef SOLIDSYSLOG_FREERTOS_LWIP_LWIPOPTS_H
#define SOLIDSYSLOG_FREERTOS_LWIP_LWIPOPTS_H

/* --- OS abstraction (NO_SYS=0: tcpip thread + FreeRTOS sys_arch) ------- */
#define NO_SYS 0
#define SYS_LIGHTWEIGHT_PROT 1
#define LWIP_TCPIP_CORE_LOCKING 1
/* Raw API only — no sequential netconn / BSD-socket API. */
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0

/* tcpip thread. Priorities are numeric here: lwipopts.h is processed via
 * lwip/opt.h before any FreeRTOS header, so configMAX_PRIORITIES (7) is not
 * visible — TCPIP_THREAD_PRIO 6 == configMAX_PRIORITIES - 1, above the
 * LAN9118 RX task (configMAX_PRIORITIES - 2 == 5, set in EthernetIf.c).
 * TCPIP_THREAD_STACKSIZE is in BYTES (LWIP_FREERTOS_THREAD_STACKSIZE_IS_
 * STACKWORDS defaults to 0, so the sys_arch divides by sizeof(StackType_t)). */
/* lwIP's api/err.c maps err_t to errno; pull the E* codes from newlib's
 * <errno.h> rather than have lwIP provide its own (which would clash with
 * newlib's definitions). */
#define LWIP_ERRNO_STDINCLUDE 1

#define TCPIP_THREAD_NAME "tcpip"
#define TCPIP_THREAD_STACKSIZE 4096
#define TCPIP_THREAD_PRIO 6
#define TCPIP_MBOX_SIZE 8
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_TCP_RECVMBOX_SIZE 8
#define DEFAULT_ACCEPTMBOX_SIZE 8

/* --- Protocol surface ------------------------------------------------- */
#define LWIP_IPV4 1
#define LWIP_IPV6 0
#define LWIP_RAW 1
#define LWIP_UDP 1
#define LWIP_TCP 1
#define LWIP_ARP 1
#define LWIP_DHCP 0
#define LWIP_DNS 0
#define LWIP_ICMP 1
#define LWIP_IGMP 0

/* etharp queues the first packet to a destination while ARP resolves it —
 * keep queueing on so the first UDP datagram after boot is not dropped
 * (mirrors the FreeRTOS-Plus-TCP first-packet ARP behaviour). */
#define ARP_QUEUEING 1

/* --- Memory: lwIP-managed static pools (no libc/posix heap) ----------- */
#define MEM_LIBC_MALLOC 0
#define MEMP_MEM_MALLOC 0
#define MEM_ALIGNMENT 4
#define MEM_SIZE (16 * 1024)

#define MEMP_NUM_UDP_PCB 4
#define MEMP_NUM_TCP_PCB 4
#define MEMP_NUM_TCP_PCB_LISTEN 2
/* TLS/mTLS handshake sizing (S28.11). A mutual-TLS handshake is the heaviest
 * traffic this target sees: it exchanges TWO certificate chains — the server's
 * inbound and the client's outbound — where plain TLS sends none from the
 * client, so its peak demand on the segment + pbuf pools (and the TCP window
 * below) is materially higher, and whether it fits depends on segment-arrival
 * timing. At the S28.09/.10 values (16/16/16 + 4*MSS) the handshake stalls; even
 * 32/32/24 left mTLS flaky. These counts are sized so a full bidirectional
 * mutual-TLS flight stays in flight without tcp_write hitting ERR_MEM (which
 * SolidSyslogLwipRawTcpStream treats as a hard send failure and tears the
 * connection down). The window/send-buffer below (6*MSS) and these counts move
 * in lockstep — lwIP derives TCP_SND_QUEUELEN from TCP_SND_BUF. Plain TLS fits a
 * smaller budget, but both transports share this one netif. */
#define MEMP_NUM_TCP_SEG 48
#define MEMP_NUM_PBUF 48
#define MEMP_NUM_RAW_PCB 2
#define MEMP_NUM_ARP_QUEUE 4
/* tcpip thread message pools: API callbacks (the marshal) + inbound packets
 * posted by the netif RX task via tcpip_input. */
#define MEMP_NUM_TCPIP_MSG_API 8
#define MEMP_NUM_TCPIP_MSG_INPKT 8

#define PBUF_POOL_SIZE 32

/* --- TCP sizing ------------------------------------------------------- */
#define TCP_MSS 1460
/* 6*MSS (was 4 in S28.09/.10) so a full mutual-TLS handshake flight — the
 * client Certificate chain + ClientKeyExchange + CertificateVerify, ~6 KB of
 * back-to-back ~2 KB records — fits the send buffer without an ERR_MEM tear-down
 * mid-handshake. See the MEMP_NUM_TCP_SEG / PBUF note above for the coupling. */
#define TCP_WND (6 * TCP_MSS)
#define TCP_SND_BUF (6 * TCP_MSS)
#define TCP_QUEUE_OOSEQ 0

/* --- Diagnostics ------------------------------------------------------ */
#define LWIP_STATS 0
#define LWIP_NETIF_API 0
#define LWIP_NETIF_STATUS_CALLBACK 0
#define LWIP_NETIF_LINK_CALLBACK 0
#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_GEN_TCP 1
#define CHECKSUM_CHECK_IP 1
#define CHECKSUM_CHECK_UDP 1
#define CHECKSUM_CHECK_TCP 1

#endif /* SOLIDSYSLOG_FREERTOS_LWIP_LWIPOPTS_H */
