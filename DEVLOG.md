# Dev Log

## 2026-03-28 — GitHub project setup

- Created `epic` label on DavidCozens/solid-syslog
- Created epic issues E0–E10 as GitHub Issues (#2–#12):
  - E0 (#2): Epic: Walking Skeleton
  - E1 (#3): Epic: Core Syslog Formatting
  - E2 (#4): Epic: UDP Transport
  - E3 (#5): Epic: TLS Transport
  - E4 (#6): Epic: Buffering
  - E5 (#7): Epic: Store and Forward
  - E6 (#8): Epic: Optional Header Fields
  - E7 (#9): Epic: Structured Data
  - E8 (#10): Epic: RTOS Examples
  - E9 (#11): Epic: C++ Wrapper
  - E10 (#12): Epic: Static Analysis and MISRA
- Created GitHub Project board "SolidSyslog" (project #1); all epics added
- No open questions

## 2026-03-28 — E0 walking skeleton completion

### Decisions
- E0 scope revised: BDD harness moved to E2, where there is something real to drive end-to-end
- E0 contains one story only: S0.1 repository setup
- README.md written with architecture summary and status notice
- SKILL.md added as standing brief for Claude Code sessions
- misra_suppressions.txt added — empty, version-controlled, ready for first suppression

### Deferred
- BDD harness (Behave/Python) — deferred to E2
- GitHub Issue story template — deferred, revisit before E1 decomposition

### Open questions
- None

## 2026-03-29 — E1 story decomposition

### Decisions
- E1 decomposed into 5 stories: E1.1–E1.5 (GitHub Issues #16–#20), created as sub-issues of Epic #3
- E1.1: Walking skeleton — single Log call produces a valid RFC 5424 message
- E1.2: PRIVAL encoding — facility and severity on the Log call
- E1.3: Timestamp — raise-time capture via injected clock function
- E1.4: Hostname, AppName, ProcId — injected via config function pointers
- E1.5: MessageId and Message — driven onto the Log call
- `story` label created (#0075ca) to distinguish stories from epics on the project board
- All stories added to SolidSyslog project board under Epic #3; Epic #3 added to board
- Structured data fixed as `-` throughout E1 — real structured data deferred to E7

### Deferred
- GitHub Issue story template — still deferred, not needed yet

### Open questions
- ~~Boundary truncation policy~~ — resolved 2026-04-01: always send, truncate strings,
  consider adjusting PRIVAL to indicate the problem. See S1.2 entry.
- ~~Out-of-range PRIVAL handling~~ — resolved 2026-04-01: override to facility 5 /
  severity 3 (PRIVAL 43), send message with remaining fields as-is. See S1.2 entry.

## 2026-03-30 — Devcontainer SSH agent bind mount fix

### Decisions
- Added `initializeCommand` to `.devcontainer/devcontainer.json` to remove stale stopped
  containers before each devcontainer open
- Command: `docker compose --project-name "$(basename $(pwd))_devcontainer" -f .devcontainer/docker-compose.yml rm -f 2>/dev/null || true`

### Root cause
Docker Desktop on Windows/WSL assigns a UUID to each WSL filesystem bind mount internally.
When a devcontainer starts, that UUID is baked into the container's stored config. After a
Windows restart, Docker Desktop resets its bind mount registry and issues new UUIDs — but
the stopped container still references the old UUID, causing:
`error mounting "/run/desktop/mnt/host/wsl/docker-desktop-bind-mounts/Ubuntu/<UUID>" ... no such file or directory`

VS Code's "Reopen in Container" tries to restart the existing stopped container (`--no-recreate`),
hitting the stale UUID. "Rebuild Container" avoids it because it creates a fresh container.
The `initializeCommand` automates the equivalent of Rebuild by dropping stopped containers
before each open — running containers are unaffected so normal reopens have no friction (~80ms overhead).

### Cross-platform note
The `initializeCommand` is safe for all contributors. `docker compose rm -f` is standard
and `$(basename $(pwd))` is POSIX — on Mac and Linux it runs harmlessly as a no-op.
The UUID issue is Windows/WSL-specific but there is no cost to running the command elsewhere.

### Deferred
- `${SSH_AUTH_SOCK}` in `docker-compose.yml` has no fallback — if a contributor has no SSH
  agent running, `docker compose` will fail with an interpolation error. A `${SSH_AUTH_SOCK:-/dev/null}`
  guard would make the container start cleanly for those contributors (SSH just won't work).
  Low priority until a second contributor joins.
- This fix should be backported to the CppUTest devcontainer template so future clones
  don't encounter the same issue.

### Open questions
- None

## 2026-03-29 — E2 story decomposition

### Decisions
- E2 renamed from "UDP Transport" to "UDP Sender" — scope is the POSIX sender implementation only
- E2 decomposed into 4 stories: E2.1–E2.4 (GitHub Issues #22–#25), created as sub-issues of Epic #4
- E2.1: Walking Skeleton — PosixUdpSender transmits a buffer
- E2.2: UdpSender configuration — host and port injection
- E2.3: CMake platform detection — PosixUdpSender included conditionally
- E2.4: BDD walking skeleton — end-to-end UDP message
- PosixUdpSender.c included in build via CMake platform detection only — no #ifdef in source
- POSIX socket calls tested with hand-rolled SocketSpy (strong-symbol fakes for socket/sendto/close) — no real network in unit tests; fff considered but rejected as an unnecessary dependency for three functions
- BDD harness deferred from E0 now lands in E2.4 as planned
- RTOS-specific sender implementations and non-POSIX build testing explicitly deferred to E8

### Deferred
- RTOS-specific sender implementations — deferred to E8
- Non-POSIX build testing — deferred to E8

### Open questions
- Port 0 behaviour: reject or default to 514? Decide before implementing E2.2
- Unresolvable hostname: error at Create time (fail fast) or at Send time (deferred failure)?
  Preferred: fail fast at Create time — matches alloc-failure pattern from S1.1. Confirm before E2.2.

## 2026-03-30 — E2 BDD infrastructure design

### Decisions
- syslog-ng chosen as BDD test oracle — RFC 5424 parser decomposes received messages into named
  fields; Behave asserts field by field, not on raw bytes. Catches conformance failures a string
  comparison would miss.
- syslog-ng runs as a third Docker Compose service in `.devcontainer/docker-compose.yml` alongside
  the existing gcc/clang devcontainer services. No Docker-in-Docker required.
- Hostname resolution in PosixUdpSender uses `getaddrinfo` — accepts both IP strings and DNS
  hostnames (e.g. `syslog-ng` service name from Docker Compose network).
- syslog-ng UDP source on port 5514 (unprivileged; no NET_BIND_SERVICE needed).
- syslog-ng output: key=value template, one line per message, written to
  `Bdd/output/received.log` via shared workspace mount. Behave reads directly.
- `flush_lines(1)` in syslog-ng destination config to minimise write latency.
- Behave test isolation via file-offset tracking: `before_scenario` records byte offset,
  steps read only new lines from that offset. No container restart between scenarios.
- Example binary (`Example/SolidSyslogExample.c`): minimal C program that creates a logger,
  sends one message via SolidSyslog+PosixUdpSender, exits. Behave invokes as subprocess.
  Lives under `Example/` as a user-facing reference, doubling as the BDD sender.
- CI: `docker compose up -d syslog-ng` before BDD step; same docker-compose.yml as devcontainer.
- SenderSpy renamed from SpySender throughout — subject-first naming more idiomatic.
- E2, S2.2, S2.3, S2.4 GitHub issues updated with infrastructure design notes.

### Deferred
- TLS cert management for RFC 5425 BDD scenarios — self-signed CA, generation script
  to be version-controlled alongside Compose config. Deferred to E3.

### Open questions
- None

## 2026-03-31 — S2.4 BDD walking skeleton implementation

### Decisions
- syslog-ng validated as BDD test oracle — receives UDP syslog on 5514, writes parsed fields
  via key=value template to `Bdd/output/received.log` with `flush_lines(1)`
- syslog-ng normalises `Z` timestamps to `+00:00` via `$ISODATE` — BDD assertions match the
  normalised form, not the sent form
- Behave container published to GHCR (`ghcr.io/davidcozens/behave`) from new repo
  `DavidCozens/BehaveDocker` — Debian trixie-slim base matches cpputest image for glibc
  compatibility, includes git and openssh-client for devcontainer use
- Devcontainer switching extended to three services: gcc, clang, behave — single `"service"`
  change in `devcontainer.json`, `postStartCommand` guards cmake behind `BUILD_PRESET` check
- Ctrl+Shift+B runs `behave Bdd/features/` when in the behave container (no `BUILD_PRESET` set)
- VS Code extensions added: `ms-python.python` and `cucumber.cucumber-official`
- CI bdd job uses artifact handoff from `build-and-test` (upload/download-artifact v4) rather
  than rebuilding — tests the same binary that passed unit tests
- CI bdd job is advisory (`continue-on-error: true`) — not a required status check
- CI bdd job uses dedicated `ci/docker-compose.bdd.yml` without dev-specific volume mounts
- syslog-ng healthcheck (`test -S /var/lib/syslog-ng/syslog-ng.ctl`) in CI compose ensures
  readiness before Behave starts
- JUnit XML output from Behave displayed via `dorny/test-reporter` in PR check runs
- Compose logs captured on failure for CI debugging
- `Example/` added to clang-format CI check
- Test isolation via line-count tracking — `Given` records line count, `When` waits for a new
  line — avoids root-owned file permission issues with truncation

### Deferred
- syslog-ng image not pinned to SHA — using `balabit/syslog-ng:latest`. Pin when stability matters.
- MISRA C:2012 addon for cppcheck — future addition, not part of this story

### Open questions
- None

## 2026-04-01 — S1.2 story rewrite and design decisions

### Decisions
- S1.2 rewritten with BDD acceptance criteria — Gherkin scenarios replace the ZOMBIES
  unit test list as the story's acceptance criteria. TDD still drives the implementation
  underneath; BDD covers the observable end-to-end behaviour.
- Out-of-range PRIVAL handling resolved: invalid facility or severity → override both to
  facility 5 (syslog) / severity 3 (err), producing PRIVAL 43. The message is still sent
  with all other fields as-is. This makes the error observable via BDD and avoids silent
  data loss.
- Boundary truncation policy resolved: the library always sends a message regardless of
  input. Strings exceeding RFC 5424 field limits (HOSTNAME 255, APP-NAME 48, MSGID 32)
  will be truncated. In each case, adjusting PRIVAL to indicate the problem will be
  considered. Error reporting deferred to a later story.
- Example program CLI: `--facility` and `--severity` flags via `getopt_long`. All fields
  default to the S1.1 test defaults when omitted. Future stories add flags for their own
  fields (e.g. `--hostname`, `--message`). This approach scales to multi-message scenarios
  needed for buffering (E4) without premature design — the example can evolve as needed.
- Existing walking skeleton BDD scenario remains unchanged — the defaults match S1.1
  values so existing tests continue to pass without modification.

### Deferred
- Error reporting API — deferred to a later story
- Additional CLI flags for other message fields — deferred to E1.3–E1.5

### Open questions
- Example program unit tests — as the example grows (more CLI flags per story), should it
  get its own unit tests? Current decision is to rely on BDD coverage, but revisit if the
  example starts containing non-trivial logic or if BDD proves too coarse to catch bugs.
