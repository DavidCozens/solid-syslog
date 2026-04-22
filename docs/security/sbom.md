# Software Bill of Materials (SBOM)

SolidSyslog publishes a [CycloneDX](https://cyclonedx.org/) 1.5 SBOM for the
shipped library. SBOMs come in three flavours that answer three different
questions — this document is only concerned with the first.

| Flavour | Question it answers | Status here |
|---|---|---|
| **Product SBOM** | "What am I linking against in my deployment?" | Covered by this workflow (see below). |
| **Build / dev-env SBOM** | "What tools, containers, and test harnesses were used to produce the release?" | Not yet — deferred to a separate story. Container image SHAs are tracked in `docs/containers.md` for now. |
| **Source SBOM** | "What third-party source code is embedded in the product?" | Empty — SolidSyslog vendors no third-party source. |

## Product SBOM scope

The product SBOM covers the `Core/` and `Platform/` subdirectories — the
source the integrator consumes from this repository. `Core/` is Tier 1
(full support, stable API); `Platform/` is Tier 2 (supported; API may
evolve per target). Both ship.

Out of scope:
- `Example/` — reference integrations, not product.
- `Tests/`, `Bdd/` — test harnesses.
- `ci/`, `docs/`, `.devcontainer/`, `.github/` — infrastructure.

Runtime dependencies we declare but do not bundle:
- **OpenSSL** — optional, only when `SOLIDSYSLOG_OPENSSL=ON`. Listed as a
  CycloneDX component with `scope: optional`. No version pinned —
  integrators select their own OpenSSL and capture it in their own SBOM
  alongside the specific licence terms of the version they ship.

Runtime dependencies we document as environment (not components):
- **POSIX libc / Winsock / POSIX message queues** — host OS APIs, not
  shipped software. Recorded as `metadata.properties` rather than
  components.

## What the SBOM says

The SBOM is a single-component document. `Core/` is a pure-C library with no
runtime dependencies — so the subject (`metadata.component`) is SolidSyslog
itself, and the top-level `components` array is empty. Runtime facts that a
deployer must supply (a POSIX or Windows host, optionally a TLS library
implementing the Stream abstraction) are documented as `properties`, not as
components — they are *requirements on the deployment*, not *shipped software*.

Key fields worth reading:

| Field | Meaning |
|---|---|
| `metadata.component.name` | `SolidSyslog`. |
| `metadata.component.version` | The value from `.release-please-manifest.json` at the time of generation. Pre-release: `0.0.0`. |
| `metadata.component.purl` | Package URL keyed to the exact commit SHA — unambiguous pointer back to the source. |
| `metadata.component.supplier.name` | `COSOSO (Cozens Software Solutions Limited)`. |
| `metadata.component.licenses[0].license.id` | `PolyForm-Noncommercial-1.0.0` — SPDX identifier. |
| `metadata.properties[solidsyslog:source-hash-sha256]` | SHA-256 of a deterministic `git archive` of the `Core/` subtree. A consumer with the same commit can independently reproduce it. |

## How to generate one (rehearsal)

Each run produces a CycloneDX 1.5 JSON file, validated against the spec by
[`cyclonedx-cli`](https://github.com/CycloneDX/cyclonedx-cli), and uploaded as
a workflow artifact.

1. Open the **Actions** tab.
2. Select the **Generate SBOM** workflow.
3. Click **Run workflow**, pick the ref (usually `main` or a release tag),
   and **Run workflow**.
4. When the run completes, scroll to **Artifacts** at the bottom of the run
   page and download `sbom-cyclonedx-<version>`.
5. Unzip; the file inside is `sbom.cdx.json`.

The workflow uses only the default `GITHUB_TOKEN` — no repo secrets required.

## Sanity-check a generated SBOM

```
cyclonedx validate --input-file sbom.cdx.json --input-format json --input-version v1_5 --fail-on-errors
```

The CI workflow already runs this; the command is useful if you've fetched
the artifact locally and want to re-verify independently.

## Deferred

Release-time publication — attaching the SBOM to every GitHub Release as an
asset, plus [sigstore/cosign](https://docs.sigstore.dev/) keyless signing
under the repository's GitHub OIDC — is the next story (S19.02). Until that
lands, this workflow is manual-only and the rendered SBOM lives only as a
workflow artifact.
