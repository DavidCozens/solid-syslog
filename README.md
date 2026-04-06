# SolidSyslog

A structured syslog client library for embedded and industrial systems, implementing
RFC 5424 (structured syslog), RFC 5426 (UDP transport), and RFC 5425 (TLS transport).

Designed for resource-constrained environments:
- C99, no dynamic memory allocation required — allocator is caller-injected
- Transport-agnostic — UDP, TLS, or bring your own
- Buffer-agnostic — NullBuffer (direct send), POSIX message queue, or bring your own
- No `#ifdef` feature flags — optional features composed at link time
- MISRA C:2012 informed
- Dependency injection throughout — fully testable without a network

## Status

Early development. Walking skeleton in place. Not yet suitable for production use.

## Building and testing

See [Building and testing](docs/builds.md).

## Architecture

SolidSyslog uses an OO-in-C style with vtable structs and dependency injection.
All fields — required and optional — use a uniform field object pattern.
Optional features are composed at link time via dead code elimination; there are
no conditional compilation directives in the library source.

Public headers are split by audience (Interface Segregation Principle):
- **`SolidSyslog.h`** — application code that logs events (`Log`, `Service`)
- **`SolidSyslogConfig.h`** — system setup code that creates and destroys loggers
- **`SolidSyslogSenderDef.h`** / **`SolidSyslogBufferDef.h`** — extension points for custom senders and buffers
- **`SolidSyslogNullBuffer.h`** — direct-send buffer for single-task systems
- **`SolidSyslogPosixMqBuffer.h`** — thread-safe POSIX message queue buffer
- **`SolidSyslogUdpSender.h`** — UDP transport
- **`SolidSyslogStructuredDataDef.h`** — extension point for custom structured data
- **`SolidSyslogMetaSd.h`** — sequenceId structured data (RFC 5424 §7.3)
- **`SolidSyslogTimeQualitySd.h`** — timeQuality structured data (RFC 5424 §7.1)
- **`SolidSyslogOriginSd.h`** — origin structured data (RFC 5424 §7.2)
- **`SolidSyslogPosixClock.h`** / **`PosixHostname.h`** / **`PosixProcId.h`** — POSIX helpers

Two example programs demonstrate usage:
- **`Example/SingleTask/`** — NullBuffer, single-task bare-metal model
- **`Example/Threaded/`** — PosixMqBuffer, two pthreads (logger + service)

## CI pipeline

See [CI pipeline](docs/ci.md).

## BDD testing

See [BDD testing](docs/bdd.md).

## Container images

See [Container images](docs/containers.md).

## License

Copyright 2026 Cozens Software Solutions Limited.

Licensed under the [PolyForm Noncommercial License 1.0.0](LICENSE.md). Free for
noncommercial, personal, educational, and government use.

For commercial licensing enquiries, please use the contact form at
[cososo.co.uk](https://www.cososo.co.uk/#contact).