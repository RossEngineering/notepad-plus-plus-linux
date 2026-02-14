# Incubator Promotion Validation (2026-02-14)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Reference criteria:

- `docs/adr/rossengineering-bafc82c7/ADR-006 — Incubator Policy & Promotion Criteria.md`
- `docs/adr/rossengineering-bafc82c7/ADR-006a Repository Promotion Checklist.md`

## Decision

Final decision: **Approved - promoted at `v0.10.0-rc.4`**.

Rationale:

1. RC train and post-RC3 maintenance execution are complete through RC4 (`v0.10.0-rc.4`).
2. RC3/GA blocker lists are operating with no open `P0` entries at promotion decision time.
3. Cross-distro integration, regression depth, and release dry-run evidence remain passing.
4. Remaining work to `v1.0.0` is GA operations and continuity closure, not incubator-maturity closure.

## Validation summary

| Area (ADR-006a) | Status | Evidence | Notes |
| --- | --- | --- | --- |
| 1. Intent & Scope | Pass | `README.md`, `docs/compatibility-target.md`, `docs/roadmap.md` | Scope and release direction are explicit and current. |
| 2. README Quality | Pass | `README.md` | Release dashboard/status are synchronized through RC4 and promoted-state text. |
| 3. Architecture & Structure | Pass | `docs/architecture.md`, `platform/include`, `platform/linux`, `ui/qt` | Boundaries and layering remain clear and documented. |
| 4. Decision Log | Pass | `docs/decisions.md`, `docs/adr/` | Decision trace remains explicit and maintained. |
| 5. Testing | Pass | `ctest` suites + RC3 regression matrix report | Regression depth now includes crash-recovery persistence and language workflow coverage. |
| 6. CI & Automation | Pass (monitoring) | `.github/workflows/linux-cmake.yml`, `.github/workflows/linux-release.yml`, desktop integration workflow | Required lanes are green; GA still tracks 7-day continuity gate. |
| 7. Versioning & Compatibility | Pass | `docs/versioning-policy.md`, `docs/compatibility-target.md` | RC/GA path and compatibility stance are explicit. |
| 8. Security Awareness | Pass | `SECURITY.md`, `docs/extension-api-v1.md` | Security reporting and extension boundary model are documented. |
| 9. Runtime & Deployment | Pass | `docs/install-linux.md`, `docs/install-consumer-linux.md`, RC3 distro validation report | Consumer install and desktop/file-handler integration are validated across target distros. |
| 10. Documentation & Maintainability | Pass | `TODO.md`, `docs/roadmap.md`, release docs/checklists | Active docs are aligned to RC4 live and GA gating. |
| 11. Final Promotion Decision | Pass | This report + `docs/releases/v0.10.0-rc.4.md` | Promotion approved and executed at RC4. |

## Post-promotion follow-through

1. Keep repository status text and governance docs aligned with promoted state.
2. Continue GA continuity and blocker closure toward `v1.0.0`.
