# RFC Compliance Matrix

SolidSyslog implements the sender (client) side of four syslog RFCs. This
document tracks which requirements are currently met, partially met, or
planned.

Status key:
- Supported — implemented and tested
- Partial — implemented with known limitations
- Planned — tracked in an issue or epic
- N/A — not applicable to a sender implementation

## RFC 5424 — The Syslog Protocol

| Section | Requirement | Status | Notes |
|---|---|---|---|
| 6.1 | PRI — facility * 8 + severity | Supported | Invalid values fall back to `syslog.err` (facility 5, severity 3) |
| 6.2.1 | VERSION = 1 | Supported | |
| 6.2.2 | TIMESTAMP — ISO 8601 with microseconds | Supported | 6-digit fractional seconds, UTC offset or `Z` |
| 6.2.2 | TIMESTAMP — NILVALUE when clock unavailable | Supported | NilClock produces `-` |
| 6.2.3 | HOSTNAME — max 255 chars, PRINTUSASCII | Partial | Truncated to 255. PRINTUSASCII validation planned — [S12.9](https://github.com/DavidCozens/solid-syslog/issues/120) |
| 6.2.4 | APP-NAME — max 48 chars, PRINTUSASCII | Partial | Truncated to 48. PRINTUSASCII validation planned — [S12.9](https://github.com/DavidCozens/solid-syslog/issues/120) |
| 6.2.5 | PROCID — max 128 chars, PRINTUSASCII | Partial | Truncated to 128. PRINTUSASCII validation planned — [S12.9](https://github.com/DavidCozens/solid-syslog/issues/120) |
| 6.2.6 | MSGID — max 32 chars, PRINTUSASCII | Partial | Truncated to 32. PRINTUSASCII validation planned — [S12.9](https://github.com/DavidCozens/solid-syslog/issues/120) |
| 6.3 | STRUCTURED-DATA — SD-ELEMENTs or NILVALUE | Supported | Extensible via `SolidSyslogStructuredData` vtable |
| 6.3 | SD-PARAM value escaping (`]`, `\`, `"`) | Not yet | Planned — [E14](https://github.com/DavidCozens/solid-syslog/issues/64) |
| 6.3.3 | timeQuality SD — tzKnown, isSynced, syncAccuracy | Supported | `SolidSyslogTimeQualitySd` |
| 6.3.4 | origin SD — software, swVersion | Supported | `SolidSyslogOriginSd`. `ip` and `enterpriseId` not implemented |
| 6.3.5 | meta SD — sequenceId | Supported | `SolidSyslogMetaSd`. Starts at 1, increments per message. `sysUpTime` and `language` not implemented |
| 6.3.5 | meta SD — sequenceId wraps at 2147483647 to 1 | Not yet | Planned — see [sequenceId rules](https://github.com/DavidCozens/solid-syslog/issues/31) |
| 6.4 | MSG — UTF-8 preferred | Partial | Passes through caller's encoding. No BOM prefix. UTF-8 safe truncation planned — [S12.10](https://github.com/DavidCozens/solid-syslog/issues/121) |
| 8.1 | Message size — max 2048 recommended | Supported | Default `SOLIDSYSLOG_MAX_MESSAGE_SIZE` = 512. Configurable via CMake |
| 9 | PRINTUSASCII in header fields (codes 33-126) | Not yet | Planned — [S12.9](https://github.com/DavidCozens/solid-syslog/issues/120) |

## RFC 5426 — Transmission of Syslog Messages over UDP

| Section | Requirement | Status | Notes |
|---|---|---|---|
| 3.1 | One message per UDP datagram | Supported | `SolidSyslogUdpSender` sends one datagram per `Send` call |
| 3.2 | Default port 514 | Supported | `SOLIDSYSLOG_UDP_DEFAULT_PORT` = 514 |
| 3.2 | Message fits in single datagram | Supported | Bounded by `SOLIDSYSLOG_MAX_MESSAGE_SIZE` |
| 3.2 | Avoid IP fragmentation (respect MTU) | Partial | Default 512 avoids fragmentation on most networks. No dynamic MTU discovery |
| 3.3 | Unreliable delivery — no confirmation | N/A | Inherent in UDP. Caller should be aware |
| 4 | No authentication/integrity/confidentiality | N/A | Use TLS transport for security — [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |

## RFC 6587 — Transmission of Syslog Messages over TCP

| Section | Requirement | Status | Notes |
|---|---|---|---|
| 3.2 | Sender initiates TCP connection | Supported | `SolidSyslogStreamSender` connects lazily on first send (S03.04) |
| 3.2 | Default port 601 | Supported | `SOLIDSYSLOG_TCP_DEFAULT_PORT = 601` per IANA assignment |
| 3.4.1 | Octet counting framing | Supported | `MSG-LEN SP MSG` prefix on every send |
| 3.4.2 | Non-transparent framing (LF trailer) | Not supported | Octet counting is the recommended method |
| 3.5 | Session closure handling | Supported | On send failure the stream is closed; the next Send transparently reconnects (S3.4) |
| 3.5 | Handle receiver-initiated close | Supported | Detected via send failure path — same reconnect-on-next-Send mechanism (S3.4) |
| 3.5 | Address rotation without app restart | Supported | App bumps `endpointVersion`; sender Disconnects and reconnects on next Send (S3.4) |
| — | Partial write handling (send returns short) | Not yet | Planned — [S12.8](https://github.com/DavidCozens/solid-syslog/issues/119) |

## RFC 5425 — TLS Transport Mapping for Syslog

| Section | Requirement | Status | Notes |
|---|---|---|---|
| 4.1 | TLS over TCP | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |
| 4.2 | Default port 6514 | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |
| 5.1 | Server certificate validation | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |
| 5.2 | Mutual TLS (client certificate) | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |
| 5.3 | TLS 1.2+ cipher suites | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |
| 5.4 | Octet counting framing (mandatory for TLS) | Planned | Will reuse existing octet counting from TCP sender |
| 5.5 | TLS close_notify handling | Planned | [E3](https://github.com/DavidCozens/solid-syslog/issues/5) |

## Summary

| RFC | Total requirements | Supported | Partial | Planned | N/A |
|---|---|---|---|---|---|
| RFC 5424 | 17 | 10 | 5 | 2 | 0 |
| RFC 5426 | 6 | 3 | 1 | 0 | 2 |
| RFC 6587 | 6 | 2 | 2 | 2 | 0 |
| RFC 5425 | 7 | 0 | 0 | 7 | 0 |
