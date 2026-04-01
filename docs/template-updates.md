# Pulling Template Updates into a Clone

When the template is updated (e.g. a new cross-compiler service, a CI hardening change,
a new CMake preset), pull those changes into any clone:

```bash
git fetch template
git merge template/main
```

Resolve any conflicts (see below), then raise a PR as normal.

## What typically conflicts

| File | Why | Resolution |
|---|---|---|
| `.release-please-manifest.json` | Version number diverges in the clone | Always keep the clone's version |
| `README.md` | Title and description are project-specific | Keep the clone's content |

## What should NOT be edited in clones

These files are owned by the template. Avoid modifying them in clones so that
template merges apply cleanly:

- `.devcontainer/docker-compose.yml` — initialised once by the init script; do not edit further
- `.github/workflows/ci.yml`
- `.github/workflows/release-please.yml`
- `CMakePresets.json`
- `Source/CMakeLists.txt`
- `Tests/CMakeLists.txt`
- `.clang-format`
- `.clang-tidy`
- `misra_suppressions.txt`
- `Interface/ExternC.h`
- `docs/` (all files)

If a change is needed to these files in a clone, consider whether it belongs in
the template first and should be merged back.
