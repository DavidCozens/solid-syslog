# MISRA C:2012 deviations

SolidSyslog is **MISRA-informed**, not certified-compliant. The project
adopts a curated subset of MISRA C:2012 rules per tier (see
`docs/NAMING.md` for the tier model). This document records every
deliberate deviation from a rule the project otherwise enforces.

Each deviation is paired with a matching entry in `misra_suppressions.txt`
(the cppcheck-misra input). The two files are complementary:

| File | Audience | Purpose |
|------|----------|---------|
| `misra_suppressions.txt` | cppcheck-misra | Machine-readable suppressions per rule / file / line |
| `docs/misra-deviations.md` | Reviewers, auditors, integrators | Why each deviation exists, with rationale, scope, approval |

Each entry in `misra_suppressions.txt` shall reference the section of
this document that authorises it. Populating the suppressions file
itself lands later in E10 — see [#12](https://github.com/DavidCozens/solid-syslog/issues/12)
(S10.03 wires cppcheck-misra into CI, S10.06 curates the rule subset
and finalises the deviation set).

The format below is patterned on MISRA's own deviation record template
(MISRA Compliance:2020 §4.2).

---

## D.001 — Rule 5.1 external identifier uniqueness relaxed to 63 characters

### Rule

> **Rule 5.1 (Required)** — External identifiers shall be distinct.

The "distinct" requirement is parameterised by the implementation's
significant-character count for external identifiers. C99 §5.2.4.1
specifies a **minimum** of 31 significant characters in external
identifiers — i.e. a conforming compiler may treat two external
identifiers that agree in the first 31 characters as the same identifier.

### Deviation

SolidSyslog requires external identifiers to be distinct in the first
**63** characters rather than the first 31.

### Scope

- **Strict tier** — `Core/Interface/`, `Core/Source/`,
  `Platform/*/Interface/`
- **Pragmatic tier** — `Platform/*/Source/`

The deviation does not apply to the Consistency-only or Out-of-scope
tiers (rule 5.1 is not enforced there at all).

### Rationale

The C99 31-character limit is a legacy linker artifact from the late
1980s. Every toolchain that SolidSyslog targets — hosted or embedded —
supports external identifiers far longer than 63 characters:

| Toolchain | Significant external-identifier characters |
|-----------|-------------------------------------------|
| GCC (any supported version)              | ≥ 1024 (effectively unlimited; capped only by symbol table length) |
| Clang / LLVM                             | ≥ 1024 (effectively unlimited) |
| MSVC 2015+                               | ≥ 2047 |
| `arm-none-eabi-gcc` (embedded GCC)       | ≥ 1024 |
| IAR Embedded Workbench (C/C++ compilers) | ≥ 200 |
| Arm Compiler 6 (Keil ARMCC 6+, LLVM-based) | ≥ 1024 |

The Tier 1 naming scheme in `docs/NAMING.md` (form
`SolidSyslogClass_Function`) routinely produces identifiers in the
40–60 character range — `SolidSyslogFreeRtosStaticResolver_Create` is
38, `SolidSyslogFreeRtosTcpStream_Destroy` is 36 — and a few public
storage-size enums approach 60 (e.g. `SOLIDSYSLOG_FREERTOSSTATICRESOLVER_SIZE`,
40). Strict 31-character distinctness would either collapse identifier
pairs that read identically up to a trailing word
(`SolidSyslogFreeRtosStaticResolver_Create` vs `_Destroy`) into a
single name, or force unidiomatic abbreviation throughout the public
API. Neither outcome serves clarity or MISRA's underlying intent
("the reader can tell two identifiers apart"); 63 characters does.

63 was chosen rather than "unlimited" so the project still names a
concrete number that every targeted toolchain comfortably exceeds. It
also matches C99's separate 63-character minimum for **internal**
identifiers (§5.2.4.1) — a single number applies project-wide.

### Risk and mitigation

- **Portability** — Constrained to toolchains that support ≥ 63
  significant characters in external identifiers. The table above
  covers every supported target; adding a new target requires verifying
  this constraint.
- **Tooling** — cppcheck-misra is configured to flag rule 5.1
  collisions at the 63-character window rather than the 31-character
  default. The configuration change is part of S10.03.
- **Review** — The naming scheme itself (see `docs/NAMING.md`,
  Tier 1) builds in a `SolidSyslog` prefix and a `Class_Function`
  shape that makes accidental 63-character collisions extremely
  unlikely. The static-analysis gate exists to catch any that slip in.

### Approval

Project owner — David Cozens. Recorded as the founding entry in this
document under [S10.01](https://github.com/DavidCozens/solid-syslog/issues/357).
