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
