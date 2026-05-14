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
supports external identifiers well in excess of 63 significant
characters:

| Toolchain | External identifier behaviour |
|-----------|-------------------------------|
| GCC (incl. `arm-none-eabi-gcc`)                | No compiler-imposed limit; identifier length is delegated to the target's linker, and all characters are significant on every linker SolidSyslog targets (ld, gold, lld, link.exe). See GCC manual, "Implementation-defined behavior". |
| Clang / LLVM (incl. Arm Compiler 6 / armclang) | Same rule as GCC for external identifiers — no compiler-imposed limit. |
| MSVC 2015+                                     | Documented maximum identifier length **2,047 characters** ([Microsoft Learn — C Identifiers](https://learn.microsoft.com/en-us/cpp/c-language/c-identifiers)). |
| IAR Embedded Workbench                         | C/C++ compiler reference manual documents an identifier limit well above 63 characters in every currently shipping version (verify on the target SKU's compiler reference for ports of SolidSyslog to non-standard SKUs). |

The Tier 1 naming scheme in `docs/NAMING.md` (form
`SolidSyslogClass_Function`) routinely produces identifiers in the
30–40 character range — `SolidSyslogFreeRtosStaticResolver_Create` is
40, `SolidSyslogFreeRtosTcpStream_Destroy` is 36 — and a few public
storage-size enums sit just below 40 (e.g.
`SOLIDSYSLOG_FREERTOSSTATICRESOLVER_SIZE`, 39). Strict 31-character
distinctness would either collapse identifier pairs that read
identically up to a trailing word
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
