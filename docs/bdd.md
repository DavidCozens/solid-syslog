# BDD Testing

Behaviour-Driven Development tests verify the example program end-to-end using
[Behave](https://behave.readthedocs.io/) (Python, Gherkin syntax).

## Architecture

The example binary (`build/debug/Example/ExampleProgram`) is the system under test.
Behave runs in a separate container, invokes the binary, and asserts on its behaviour
(currently: exit code).

```
Bdd/
  features/
    example.feature        — Gherkin scenarios
    steps/example_steps.py — Step definitions
    environment.py         — Behave hooks (sets binary path from env)
  output/                  — Runtime output (gitignored)
  junit/                   — JUnit XML results (gitignored)
ci/
  docker-compose.bdd.yml   — Minimal compose for CI (no dev mounts)
```

## Running locally

### From the gcc devcontainer

Build the example first, then run behave via the compose behave service:

```bash
cmake --build --preset debug --target ExampleProgram
docker compose -f .devcontainer/docker-compose.yml run --rm behave \
    behave Bdd/features/
```

### From the behave devcontainer

Switch `"service"` to `"behave"` in `.devcontainer/devcontainer.json`, rebuild, then
`Ctrl+Shift+B` runs behave directly. The example binary must already be built (by the
gcc container).

## Running in CI

The `bdd` job in `.github/workflows/ci.yml`:

1. Downloads the example binary artifact from `build-and-test`
2. Runs `docker compose -f ci/docker-compose.bdd.yml up`
3. Reports results via `dorny/test-reporter`

The BDD job is **advisory** (`continue-on-error: true`) — it does not block merge.

## Adding scenarios

1. Add a `.feature` file under `Bdd/features/`
2. Implement step definitions in `Bdd/features/steps/`
3. Run locally to verify, then push — CI will pick it up automatically

## Customising for cloned projects

When cloning the template for a new component:

- Replace `example.feature` and `example_steps.py` with project-specific scenarios
- Update `environment.py` if the binary path or environment variables change
- Add any additional Docker Compose services (e.g. test oracles) to
  `.devcontainer/docker-compose.yml` and `ci/docker-compose.bdd.yml`
