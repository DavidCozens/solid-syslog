# CI Pipeline

GitHub Actions runs all jobs in parallel on every push and pull request to `main`.

## Jobs

| Job | Preset | Notes |
|---|---|---|
| `build-and-test` | `debug` | Test results annotated on PR |
| `clang-build-and-test` | `clang-debug` | Second compiler check using Clang 19 |
| `sanitize` | `sanitize` | ASan + UBSan — test results annotated on PR |
| `coverage` | `coverage` | Summary in Actions UI; HTML report deployed to GitHub Pages on merge to `main` |
| `tidy` | `tidy` | clang-tidy — pass/fail with errors in job log |
| `cppcheck` | `cppcheck` | cppcheck static analysis |
| `format` | — | clang-format dry-run; fails if any file needs reformatting |
| `bdd` | — | End-to-end BDD test via Docker Compose (syslog-ng + Behave). Advisory — not a required check |

## Branch protection

All jobs except `bdd` are required status checks. A PR cannot be merged unless the required checks pass.
The `bdd` job runs and reports pass/fail but does not block merge (`continue-on-error: true`).
Direct pushes to `main` are blocked. Squash merge only.

## Release automation

[release-please](https://github.com/googleapis/release-please) runs on every push to `main`.
It reads commit messages (which must follow [Conventional Commits](https://www.conventionalcommits.org/))
and maintains a release PR that bumps the version and updates `CHANGELOG.md`.
Merging that PR creates a GitHub Release and tag.

## Permissions

Each job is granted only the permissions it needs. The default token scope is
`contents: read`. Jobs that publish test results additionally hold `checks: write`
and `pull-requests: write`. The coverage job additionally holds `pages: write` and
`id-token: write` for GitHub Pages deployment.
