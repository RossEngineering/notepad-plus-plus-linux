# Roadmap

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This timeline reflects project direction as of February 14, 2026.
`v1.0.0` is published and marked latest.
Current focus is post-GA feature delivery.

## Milestones

| Milestone | Target | Scope | Status |
| --- | --- | --- | --- |
| M7: General Availability (`v1.0.0`) | Completed | GA blockers closed, release cut, and publication complete. | Done |
| M8: Post-GA Wave 1 (`v1.1.x`) | Current | Editor productivity and workflow quality improvements. | In Progress |
| M9: Post-GA Wave 2 (`v1.2.x`) | Next | Language intelligence and formatter ecosystem maturity. | Planned |
| M10: Post-GA Wave 3 (`v1.3.x`) | Next | Linux packaging/distribution hardening and install lifecycle polish. | Planned |
| M11: Post-GA Wave 4 (`v1.4.x`) | Next | Reliability, observability, and scale/soak quality gates. | Planned |

## GA closeout gate (`v1.0.0`)

1. Zero open `P0` defects in `docs/ga-blockers.md`.
2. Linux required CI lanes green on the selected GA candidate commit.
   - 7-day continuity gate is vetoed for `v1.0.0` only (`docs/ga-gate-exception-2026-02-14.md`).
3. No unresolved data-loss or crash-recovery regressions.
4. `docs/releases/v1.0.0.md` and `docs/releases/v1.0.0-checklist.md` finalized.
5. Go/no-go review updated and approved in `docs/v1.0.0-go-no-go-review-2026-02-14.md`.

GA status: complete (`https://github.com/RossEngineering/notepad-plus-plus-linux/releases/tag/v1.0.0`).

## Post-GA roadmap proposal

### M8 - Editor UX and workflows (`v1.1.x`)

1. Ship dockable layout presets and persistent panel/workspace profiles.
2. Ship shortcut profile presets + import/export for team portability.
3. Improve search/replace workflows (saved queries, grouped results, scope history).
4. Expand session templates and startup modes for common project types.
5. Add customizable status bar modules for fast context switching.

### M9 - Language intelligence and formatting (`v1.2.x`)

1. Add robust formatter orchestration (timeouts, retries, deterministic fallbacks).
2. Add project-local language/formatter overrides.
3. Expand code-action UX with preview + selective apply support.
4. Increase built-in language defaults and smarter auto-detect confidence handling.
5. Improve diagnostics navigation and inline quick-fix flow.

### M10 - Linux integration and distribution (`v1.3.x`)

1. Harden package publish/sign/verify pipeline for Arch/DEB/RPM outputs.
2. Add distro-specific install/upgrade/uninstall playbooks.
3. Add channelized updates (stable/candidate/nightly) with safe rollback path.
4. Improve sandbox/Desktop Portal behavior for Flatpak/Snap workflows.
5. Add self-heal flow for file associations and launcher integration.

### M11 - Reliability and observability (`v1.4.x`)

1. Add local diagnostic bundle export for bug-report attachment quality.
2. Expand regression suites for large files, mixed encodings, and extension lifecycle.
3. Add soak/performance CI lanes with explicit budgets and alert thresholds.
4. Harden config migration across multiple version hops.
5. Strengthen crash-recovery and extension-host restart semantics.

## Delivery cadence

1. Execute work by roadmap stream section.
2. At each section close, publish a tagged prerelease and corresponding release docs.
3. Promote selected prerelease trains into stable tags when gate criteria pass.
4. Keep `master` releasable; use `ga-dev-weekN` branches for forward batching.

## Reference artifacts

1. GA blockers: `docs/ga-blockers.md`
2. GA continuity evidence: `docs/ga-ci-continuity-log-2026-02.md`
3. GA release notes/checklist: `docs/releases/v1.0.0.md`, `docs/releases/v1.0.0-checklist.md`
4. Go/no-go review: `docs/v1.0.0-go-no-go-review-2026-02-14.md`
5. Pre-GA TODO archive: `docs/todo-archive-pre-ga-2026-02-14.md`
