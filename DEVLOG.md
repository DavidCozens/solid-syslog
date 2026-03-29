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
- Boundary truncation policy: when field values exceed RFC 5424 max length (HOSTNAME 255,
  APP-NAME 48, MSGID 32), should the library truncate silently or return an error?
  Raised in E1.2, E1.4, E1.5 — decision needed before implementation of those stories
- Out-of-range PRIVAL handling: invalid facility or severity values — clamp or reject?
  Raised in E1.2 — decision needed before implementation
