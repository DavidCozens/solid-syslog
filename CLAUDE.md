# Claude Code Guidelines

## Git Workflow

All changes to `main` must go via a pull request — direct pushes are blocked by branch protection.

**Branch naming:** `<type>/<short-description>` — e.g. `feat/clang-preset`, `ci/pin-action-shas`

**Merge strategy:** Squash merge only. This keeps a linear history on `main` and means the PR title
becomes the single commit message — so the PR title must follow Conventional Commits format (see below).

**Before raising a PR:**
- All CI checks must pass: build-and-test, clang-build-and-test, sanitize, coverage, tidy, cppcheck, format
- Commits on the branch can be informal (work-in-progress messages are fine)
- The PR title is what matters — it becomes the permanent commit message on `main`

**Branch protection rules (configured on GitHub):**
- Direct pushes to `main` are blocked
- PRs require all status checks to pass before merging: build-and-test, clang-build-and-test, sanitize, coverage, tidy, cppcheck, format
- Squash merge only — other merge strategies are disabled
- Branches are deleted automatically after merge

When cloning this template, reconfigure these branch protection rules on the new repository.

---

## Commit Messages

All commit messages must follow [Conventional Commits](https://www.conventionalcommits.org/) format.
This drives automated changelog generation and release versioning via release-please.

```
<type>[!]: <description>

[optional body]
```

| Type | Use for |
|---|---|
| `feat` | New functionality |
| `fix` | Bug fix |
| `ci` | CI/build/tooling changes |
| `refactor` | Code restructuring without behaviour change |
| `chore` | Maintenance (e.g. container image bump) |
| `docs` | Documentation only |

Append `!` for breaking changes: `feat!: rename Example target`.

PR titles must also follow this format — on squash merge the PR title becomes the commit message.

---

## TDD Discipline

Follow Uncle Bob's Three Rules of TDD strictly — **red/green/refactor in strict order**:

- **Red**: write the simplest failing test. Use inline literal values — do not introduce named constants
  or helpers at this step. Compilation failures count as failures.
- **Green**: write the minimum production code to pass the test. Hard-coded values are correct here.
- **Refactor**: while green, extract named constants, helpers, and DRY improvements. This is the right
  time to introduce `TEST_*` constants, field index constants, and test helpers.

### Test Defaults Pattern

For walking skeleton stories, use hard-coded "test default" values that are obviously fake
(e.g. `TestHost`, `42`, the RFC 5424 publication date `2009-03-23T00:00:00.000Z`). Name them `TEST_*`.
These are baked into production code initially; later stories drive real values in via config injection.

Named constants and test helpers emerge through the refactor step — never introduced upfront.

### Three Laws

1. You may not write production code unless it is to make a failing unit test pass.
2. You may not write more of a unit test than is sufficient to fail — compilation failures are failures.
3. You may not write more production code than is sufficient to pass the one failing unit test.

Refactoring must follow SOLID and DRY principles:
- **Single Responsibility** — one reason to change per module/class
- **Open/Closed** — open for extension, closed for modification
- **Liskov Substitution** — subtypes must be substitutable for their base types
- **Interface Segregation** — prefer narrow, focused interfaces
- **Dependency Inversion** — depend on abstractions, not concretions
- **DRY** — every piece of knowledge has a single, authoritative representation

The target is 100% line and branch coverage. The CI gate is 90% — if coverage drops below that, the build fails.
If 100% is proving difficult to achieve, the first response should be to reconsider the design, not lower the bar.
In practice, following TDD strictly means 100% is the natural outcome. Exceptions exist but are rare; if you find
yourself needing one, discuss the design first.

---

## CMake Presets

| Preset | Purpose |
|---|---|
| `debug` | Standard debug build — primary development preset |
| `clang-debug` | Clang build — portability check against GCC |
| `sanitize` | ASan + UBSan — run regularly during development |
| `coverage` | lcov/genhtml — 100% line and branch required |
| `tidy` | clang-tidy — all warnings treated as errors |
| `cppcheck` | cppcheck static analysis |
| `release` | Release build — optimisations enabled, no instrumentation |

Build and test: `cmake --preset <name> && cmake --build --preset <name> --target junit`
Coverage report: `cmake --preset coverage && cmake --build --preset coverage --target coverage`

---

## Project Structure

```
Interface/   — Public headers only. No implementation. This is the API boundary.
Source/      — Implementation. Compiled into a static library.
Tests/       — CppUTest unit tests. Never link production code directly; always via the library.
Example/     — Example program. Links against the library; also used as the BDD sender.
Bdd/         — BDD test infrastructure: Gherkin features, step definitions, syslog-ng config.
ci/          — CI-specific files (e.g. docker-compose.bdd.yml).
```

The separation between `Interface/` and `Source/` is deliberate — it enforces the dependency inversion
boundary that makes the code testable and portable to embedded targets.

### Public header audiences (Interface Segregation)

Headers in `Interface/` are split by audience — each user includes only what they need:

| Header | Audience | Provides |
|---|---|---|
| `SolidSyslog.h` | Application code that logs events | `SolidSyslogMessage`, `SolidSyslog_Log` |
| `SolidSyslogConfig.h` | System setup code | `SolidSyslogConfig`, `SolidSyslog_Create`, `SolidSyslog_Destroy` |
| `SolidSyslogPrival.h` | Any code that needs facility/severity enums | `SolidSyslog_Facility`, `SolidSyslog_Severity` |
| `SolidSyslogTimestamp.h` | Any code that needs the timestamp struct | `SolidSyslogTimestamp`, `SolidSyslogClockFunction` |
| `SolidSyslogSenderDef.h` | Sender implementors (extension point) | `SolidSyslogSender` vtable struct |
| `SolidSyslogUdpSender.h` | System setup code using UDP transport | `SolidSyslogUdpSender_Create`, `_Destroy` |
| `SolidSyslogPosixClock.h` | System setup code using POSIX clock | `SolidSyslogPosixClock_GetTimestamp` |

Most application code only needs `SolidSyslog.h` — it never sees allocators, senders, or config structs.

---

## Naming Conventions

| Element | Convention | Example |
|---|---|---|
| C public functions | `PascalCase_PascalCase` | `LedDriver_TurnOn()` |
| C static/private functions | `PascalCase` | `CalculateChecksum()` |
| C++ methods | `camelCase` | `getValue()` |
| Variables | `camelCase` | `sensorReading` |
| Types / Classes / Structs | `PascalCase` | `MotorController` |
| Macros / Constants | `UPPER_SNAKE_CASE` | `MAX_BUFFER_SIZE` |
| Files | `PascalCase` | `LedDriver.c` |

No Hungarian notation. No member variable prefixes (`m_`, `_`, etc.).
Names should be self-documenting — prefer clarity over brevity.

---

## Code Style

- Formatting is enforced by clang-format. Run format-on-save or `clang-format -i` before committing.
  CI will reject unformatted code.
- clang-tidy checks are configured in `.clang-tidy`. All warnings are errors.
- All compiler warnings are errors (`-Werror`). Do not suppress warnings without strong justification.
- cppcheck runs with `--error-exitcode=1`. Inline suppressions (`// cppcheck-suppress`) must include
  a comment explaining why.

---

## Container Images

See [`docs/containers.md`](docs/containers.md) for the full image reference, Docker Compose setup,
and switching procedure.

When updating an image:

1. Build and push the new image in the container image repo
2. Update the SHA tag in all files that reference it (see [`docs/containers.md`](docs/containers.md) for the full list)
3. Rebuild the devcontainer and verify the new tooling works locally
4. Then commit — use `chore: bump container image to <sha>`
