# Incubator Promotion Validation (2026-02-13)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Reference criteria:

- `docs/adr/rossengineering-bafc82c7/ADR-006 — Incubator Policy & Promotion Criteria.md`
- `docs/adr/rossengineering-bafc82c7/ADR-006a Repository Promotion Checklist.md`

## Decision

Current recommendation: **Remain Incubator (Defer Promotion)**.

Rationale:

1. Promotion checklist evidence is strong in core engineering areas (tests, CI, decision log, release automation).
2. Promotion confidence is currently limited by remaining RC-cycle risk (RC1 in progress and deeper RC2/RC3 integration work still ahead).
3. Promotion should be reassessed once RC1 quality gates are green on tag and RC2 scope hardening is underway.

## Validation summary

| Area (ADR-006a) | Status | Evidence | Notes |
| --- | --- | --- | --- |
| 1. Intent & Scope | Pass | `README.md`, `docs/compatibility-target.md`, `docs/roadmap.md` | Purpose, scope, and non-goals are documented. |
| 2. README Quality | Partial | `README.md` | Incubator honesty is good, but release-status text is stale/inconsistent with latest state. |
| 3. Architecture & Structure | Pass | `docs/architecture.md`, `platform/include`, `platform/linux`, `ui/qt`, `CMakeLists.txt` | Clear boundaries and defendable architecture decisions exist. |
| 4. Decision Log | Pass | `docs/decisions.md`, `docs/adr/` | Decisions and ADR traceability are present. |
| 5. Testing | Pass (with caveat) | `CMakeLists.txt` test targets, `ctest --preset debug` (12/12 pass on 2026-02-13) | Automated tests exist and run; continue increasing depth as planned in RC work. |
| 6. CI & Automation | Pass (with caveat) | `.github/workflows/linux-cmake.yml`, `.github/workflows/win32-boundary.yml`, `.github/workflows/linux-release.yml` | Required pipelines exist and are committed; promotion should require recent green evidence at decision time. |
| 7. Versioning & Compatibility | Pass | `docs/versioning-policy.md`, `docs/compatibility-target.md` | Versioning and compatibility stance are explicit. |
| 8. Security Awareness | Pass | `SECURITY.md`, `docs/extension-api-v1.md` | Security reporting path and boundary model are present. |
| 9. Runtime & Deployment | Pass (with caveat) | `BUILD.md`, `packaging/arch/PKGBUILD`, `docs/release-publishing.md`, `docs/distro-validation-plan.md`, `docs/distro-validation-report-rc1-2026-02-13.md` | Required RC1 distro baseline evidence is present; continue broadening packaging confidence in RC2+. |
| 10. Documentation & Maintainability | Pass (with caveat) | `docs/roadmap.md`, `TODO.md`, `docs/rc1-blockers.md`, `docs/releases/v0.9.3-rc.1.md` | Planning and release docs are now aligned; keep status synced as RC train advances. |
| 11. Final Promotion Decision | Not ready | This report + open blocker docs | Promotion should be revisited after RC1 blockers and distro evidence are closed. |

## Hard blockers before promotion

1. Run promotion check at a known green point (all required CI lanes passing on current RC candidate SHA/tag).
2. Keep `SUPPORTED_SYSTEM.md` and release-facing status docs aligned with current RC milestone.
3. Reassess incubator status before RC3 start, targeting promotion readiness before GA.

## Fresh verification run included in this validation

1. `ctest --preset debug --output-on-failure`:
   - Result: pass (`12/12`).
2. `./scripts/check_win32_boundaries.sh HEAD`:
   - Result: pass (`no new Win32 usage outside platform/win32`).

## Promotion re-check trigger

Re-run this validation when:

1. RC1 tag candidate has green required CI + sanitizer lanes.
2. RC1 release decision (go/no-go) is recorded.
3. RC2 hardening scope is confirmed.

At that point, perform an explicit promote/defer decision record.
