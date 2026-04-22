# Release Verification Guide

You have a SolidSyslog release on disk and want to convince yourself, or an
auditor, that it was produced by the repo it claims to come from. This guide
walks through the verification a careful integrator would run, in the order
they'd naturally run it.

Prerequisites:
- [cosign](https://docs.sigstore.dev/system_config/installation/) v2 or
  later on your `$PATH`.
- [cyclonedx-cli](https://github.com/CycloneDX/cyclonedx-cli) v0.30.0 or
  later (optional — only needed to re-validate the SBOM).
- A `git` checkout of the repo at the release's tag (optional — only
  needed to reproduce the source hash yourself).

All four Release assets should be present:

```text
sbom.cdx.json
sbom.cdx.json.bundle
source-sha256.txt
source-sha256.txt.bundle
```

## 1. Verify the source is what we claim

`source-sha256.txt` contains one line: the SHA-256 of
`git archive --format=tar.gz HEAD -- Core/ Platform/` at the release tag.
To reproduce:

```shell
git clone --depth 1 --branch v<version> https://github.com/DavidCozens/solid-syslog.git
cd solid-syslog
git archive --format=tar.gz HEAD -- Core/ Platform/ | sha256sum
```

Compare the hash to the one in `source-sha256.txt`. If they match, the source
you just cloned is byte-identical to the source the SBOM describes.

(Note: `git archive` output can vary slightly across git versions — if the
hash doesn't match, try a recent git; if it still doesn't match, cross-check
the content against the SBOM's `metadata.properties[solidsyslog:commit-sha]`
property and inspect the source directly.)

## 2. Verify the SBOM signature

The signature commits to the GitHub Actions workflow that produced it.
A valid signature proves three things:

1. This exact SBOM came from the SolidSyslog repo's `sbom.yml` workflow.
2. The workflow ran at the release tag you think it did (not on a random
   branch, not on a fork).
3. The signing event was logged to the Sigstore transparency log at the
   time claimed.

Verification:

```shell
cosign verify-blob \
  --bundle sbom.cdx.json.bundle \
  --certificate-identity "https://github.com/DavidCozens/solid-syslog/.github/workflows/sbom.yml@refs/tags/v<version>" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  sbom.cdx.json
```

`cosign` outputs `Verified OK` and exits 0 on success. Any of these
conditions fail the verification loudly:

- The SBOM has been modified after signing.
- The signature was produced by a different workflow file.
- The signature was produced on a different repo (e.g. a fork).
- The signature was produced against a different tag.
- The Sigstore transparency log entry is missing or doesn't match.

## 3. Verify the source-hash signature

```shell
cosign verify-blob \
  --bundle source-sha256.txt.bundle \
  --certificate-identity "https://github.com/DavidCozens/solid-syslog/.github/workflows/sbom.yml@refs/tags/v<version>" \
  --certificate-oidc-issuer "https://token.actions.githubusercontent.com" \
  source-sha256.txt
```

Same guarantees as step 2, but for the source-hash file. Combined with the
hash match from step 1, you now know the source you have is the source the
SBOM describes, and the SBOM is the one the workflow produced.

## 4. (Optional) Re-validate the SBOM against CycloneDX

```shell
cyclonedx validate \
  --input-file sbom.cdx.json \
  --input-format json \
  --input-version v1_5 \
  --fail-on-errors
```

This isn't strictly a provenance check — the CI already ran it — but it
confirms the document on your disk is structurally and semantically a valid
CycloneDX 1.5 SBOM. Useful if you're plugging it into an SBOM-consuming tool
and want to rule out corruption-in-transit.

## What verification does *not* tell you

- It doesn't tell you whether the code behind the SBOM is bug-free, secure,
  or fit for purpose. The SBOM is provenance evidence, not a quality
  claim.
- It doesn't tell you anything about dependencies that SolidSyslog chose
  not to bundle. OpenSSL appears in the SBOM's `components[]` with
  `scope: optional`, but picking and verifying your OpenSSL is your job.
- It doesn't tell you whether the SolidSyslog licence is compatible with
  your intended use. That's a licence review, not a signature check.

See `docs/iec62443.md` for the security posture, and `docs/security/sbom.md`
for a walk-through of what the SBOM actually says.
