# Vulnerability Triage Runbook

This is the maintainer's operational counterpart to [`SECURITY.md`](../../SECURITY.md):
how a vulnerability report is handled end-to-end, from receipt to retrospective.
`SECURITY.md` states the public promises (72-hour acknowledgement, 90+14
disclosure); this runbook is how they are met.

**The single tracking record for any vulnerability is its draft GitHub Security
Advisory (GHSA).** No separate issue, spreadsheet, or external tracker — the draft
GHSA holds the timeline, severity, affected/fixed versions, and reporter details
from receipt through publication.

Reports arrive via the channels in `SECURITY.md`: GitHub private vulnerability
reporting (which opens a draft GHSA directly) or the `cososo.co.uk/security/report`
web form.

## Stage 1 — Receipt (hours 0–72)

- [ ] Log the report by opening (or confirming) a **draft GHSA** on the repo.
- [ ] If it came via the web form, an auto-acknowledgement has already gone out;
      send a **human acknowledgement within 72 hours** regardless.
- [ ] Capture the reporter's contact and their **credit preference** (name/handle,
      or anonymous) in the advisory.
- [ ] Do not confirm or deny severity yet — that's triage.

## Stage 2 — Triage (days 0–7)

- [ ] **Scope** the issue against the support tiers (see `SECURITY.md`):
  - **Tier 1 `Core/`** — full treatment (CVE, advisory, fix, signed release).
  - **Tier 2 `Platform/`** / **Tier 3 `Bdd/Targets/`** — advisory only, no CVE
    unless the root cause reaches into `Core/`.
  - **Out of repo** (integrator code, a linked TLS/crypto library, the OS) —
    redirect; see *Non-standard reports* below.
- [ ] **Reproduce** where feasible; record the reproduction in the advisory.
- [ ] Assign a **CVSS v3.1** vector and score; derive the qualitative band.
- [ ] For a `Core/` issue, **request a CVE** via GitHub's CNA from the advisory.
- [ ] Decide the fix workflow by severity:
  - **High / Critical** → private: develop the fix in the GHSA's **private fork**,
    publish the advisory coordinated with the release.
  - **Low / Medium** → open: fix in a normal PR; the advisory publishes when the
    release ships.
- [ ] Update the reporter with the triage outcome and expected next step.

## Stage 3 — Fix development

- [ ] **Regression test first** — a failing test that captures the vulnerability,
      per the project's TDD discipline. This becomes the permanent guard.
- [ ] Implement the minimal fix to pass it.
- [ ] Use a **Conventional Commit** (`fix:` …) so release-please picks it up and
      the CHANGELOG entry is generated.
- [ ] High/Critical: keep this on the private fork until release is coordinated.

## Stage 4 — Release coordination

See [`release-process.md`](../release-process.md) for the mechanics; the
security-specific steps are:

- [ ] Merge the fix to `main` (from the private fork for High/Critical).
- [ ] Merge release-please's release PR to cut the tagged release.
- [ ] Confirm the `release.published` workflow attached the **SBOM + signatures**.
- [ ] **Publish the GHSA** coordinated with the release going live.
- [ ] Edit the release notes to reference the **GHSA / CVE**.

## Stage 5 — Post-release

- [ ] Notify the reporter that the fix has shipped.
- [ ] **Credit** the reporter per the consent captured at intake.
- [ ] Close the tracking advisory.

## Stage 6 — Retrospective

A brief post-mortem, captured as a comment on the advisory:

- [ ] How did the bug get in? Did existing tests miss it, and why?
- [ ] Are there **similar-class issues** elsewhere in the code to sweep proactively?
- [ ] Any process gap this response exposed?

## Reporter communication

Update the reporter at **each stage transition** — receipt, triage outcome, fix
in progress, release shipped. Silence is the most common complaint in coordinated
disclosure; a short "still on it" beats nothing.

## Non-standard reports

- **Out of scope / not a vulnerability / intended behaviour** — explain the
  reasoning, point to the relevant docs (e.g. the threat model's *caller
  obligations*), and thank the reporter. Close the advisory as not-applicable.
- **Root cause in an upstream dependency** (a linked TLS/crypto library) —
  redirect the reporter to that project's disclosure process; SolidSyslog bundles
  none of them. Track only if `Core/` needs a compensating change.
- **Self-reported** (you find it yourself) — same flow, no external reporter; skip
  the acknowledgement/credit steps.

## Maintainer unavailability

The public timelines are best-effort (see `SECURITY.md`'s force-majeure clause).
When unavailable:

- **Short-term** (days) — covered by the stated SLAs plus the force-majeure clause.
- **Medium-term** (illness, travel) — post a brief holding note on the repo or
  `cososo.co.uk`; hand off to a cover arrangement if one exists.
- **Long-term** (unable to continue) — triggers the **continuity commitment**: the
  current release is relicensed under a permissive OSS licence so users aren't
  stranded (see `SECURITY.md`).

## Evidence retention

No separate archive is required. GitHub retains, indefinitely, everything that
constitutes the diligence record: published GHSAs, release assets, SBOMs,
signatures, and commit history. This is the evidence store a CRA-regulated
integrator can cite — it satisfies the expectation of retained technical
documentation without any manual step.
