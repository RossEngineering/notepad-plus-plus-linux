# Incubator Promotion Validation (2026-02-13)

Reference criteria:

- `docs/adr/rossengineering-bafc82c7/ADR-006 — Incubator Policy & Promotion Criteria.md`
- `docs/adr/rossengineering-bafc82c7/ADR-006a Repository Promotion Checklist.md`

## Decision

Current recommendation: **Remain Incubator (Defer Promotion)**.

Rationale:

1. Promotion checklist evidence is strong in core engineering areas (tests, CI, decision log, release automation).
2. Promotion confidence is currently limited by open RC1 blockers and incomplete cross-distro validation evidence.
3. Repository-facing status documentation still has a few inconsistencies that should be corrected before promotion.

## Validation summary

| Area (ADR-006a) | Status | Evidence | Notes |
| --- | --- | --- | --- |
| 1. Intent & Scope | Pass | `README.md`, `docs/compatibility-target.md`, `docs/roadmap.md` | Purpose, scope, and non-goals are documented. |
| 2. README Quality | Partial | `README.md` | Incubator honesty is good, but release-status text is stale/inconsistent with latest state. |
| 3. Architecture & Structure | Pass | `docs/architecture.md`, `platform/include`, `platform/linux`, `ui/qt`, `CMakeLists.txt` | Clear boundaries and defendable architecture decisions exist. |
| 4. Decision Log | Pass | `docs/decisions.md`, `docs/adr/` | Decisions and ADR traceability are present. |
| 5. Testing | Pass (with caveat) | `CMakeLists.txt` test targets, `ctest --preset debug` (11/11 pass on 2026-02-13) | Automated tests exist and run; continue increasing depth as planned in RC work. |
| 6. CI & Automation | Pass (with caveat) | `.github/workflows/linux-cmake.yml`, `.github/workflows/win32-boundary.yml`, `.github/workflows/linux-release.yml` | Required pipelines exist and are committed; promotion should require recent green evidence at decision time. |
| 7. Versioning & Compatibility | Pass | `docs/versioning-policy.md`, `docs/compatibility-target.md` | Versioning and compatibility stance are explicit. |
| 8. Security Awareness | Pass | `SECURITY.md`, `docs/extension-api-v1.md` | Security reporting path and boundary model are present. |
| 9. Runtime & Deployment | Partial | `BUILD.md`, `packaging/arch/PKGBUILD`, `docs/release-publishing.md`, `docs/distro-validation-plan.md` | Arch-family path is strong; broader distro execution evidence is not complete yet. |
| 10. Documentation & Maintainability | Partial | `docs/roadmap.md`, `TODO.md`, `docs/rc1-blockers.md` | Planning is strong, but some user-facing docs remain out of date/inherited. |
| 11. Final Promotion Decision | Not ready | This report + open blocker docs | Promotion should be revisited after RC1 blockers and distro evidence are closed. |

## Hard blockers before promotion

1. Close open `P0` RC1 blockers in `docs/rc1-blockers.md` (especially cross-distro baseline and permissions UX hardening).
2. Complete and attach required distro validation evidence for Arch derivatives, Ubuntu LTS, and Fedora stable:
   - `docs/distro-validation-report-rc1-2026-02-13.md` currently has pending sections.
3. Correct stale repository-facing status documentation:
   - `README.md` still frames Beta 2 as a target rather than completed.
4. Replace or refactor inherited `SUPPORTED_SYSTEM.md` content so support statements are Linux-fork accurate.
5. Run promotion check at a known green point (all required CI lanes passing on current `master`/candidate SHA).

## Fresh verification run included in this validation

1. `ctest --preset debug --output-on-failure`:
   - Result: pass (`11/11`).
2. `./scripts/check_win32_boundaries.sh HEAD`:
   - Result: pass (`no new Win32 usage outside platform/win32`).

## Promotion re-check trigger

Re-run this validation when:

1. RC1 blocker list shows zero unresolved `P0`.
2. Distro validation report has completed evidence for all required RC1 lanes.
3. Documentation consistency fixes are merged.

At that point, perform an explicit promote/defer decision record.
