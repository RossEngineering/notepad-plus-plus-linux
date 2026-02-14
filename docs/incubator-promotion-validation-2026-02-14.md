# Incubator Promotion Validation (2026-02-14)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Reference criteria:

- `docs/adr/rossengineering-bafc82c7/ADR-006 — Incubator Policy & Promotion Criteria.md`
- `docs/adr/rossengineering-bafc82c7/ADR-006a Repository Promotion Checklist.md`

## Decision

Current recommendation: **Promotion Ready (Approve at GA go/no-go)**.

Rationale:

1. RC train execution is complete through RC3 (`v0.9.9-rc.3` published).
2. RC3 blocker list is closed with no unresolved `P0` items.
3. Cross-distro integration, full regression matrix, and release dry-run evidence are recorded and passing.
4. Remaining work to `v1.0.0` is release-operations closure, not incubator-maturity closure.

## Validation summary

| Area (ADR-006a) | Status | Evidence | Notes |
| --- | --- | --- | --- |
| 1. Intent & Scope | Pass | `README.md`, `docs/compatibility-target.md`, `docs/roadmap.md` | Scope and release direction are explicit and current. |
| 2. README Quality | Pass | `README.md` | Release dashboard/status are synchronized through RC3 and GA-in-progress state. |
| 3. Architecture & Structure | Pass | `docs/architecture.md`, `platform/include`, `platform/linux`, `ui/qt` | Boundaries and layering remain clear and documented. |
| 4. Decision Log | Pass | `docs/decisions.md`, `docs/adr/` | Decision trace remains explicit and maintained. |
| 5. Testing | Pass | `ctest` suites + RC3 regression matrix report | Regression depth now includes crash-recovery persistence and language workflow coverage. |
| 6. CI & Automation | Pass (monitoring) | `.github/workflows/linux-cmake.yml`, `.github/workflows/linux-release.yml`, desktop integration workflow | Required lanes are green; GA still tracks 7-day continuity gate. |
| 7. Versioning & Compatibility | Pass | `docs/versioning-policy.md`, `docs/compatibility-target.md` | RC/GA path and compatibility stance are explicit. |
| 8. Security Awareness | Pass | `SECURITY.md`, `docs/extension-api-v1.md` | Security reporting and extension boundary model are documented. |
| 9. Runtime & Deployment | Pass | `docs/install-linux.md`, `docs/install-consumer-linux.md`, RC3 distro validation report | Consumer install and desktop/file-handler integration are validated across target distros. |
| 10. Documentation & Maintainability | Pass | `TODO.md`, `docs/roadmap.md`, release docs/checklists | Active docs are aligned to RC3 live and GA gating. |
| 11. Final Promotion Decision | Ready | This report + GA go/no-go review | Record explicit promote/defer decision as part of final `v1.0.0` sign-off. |

## Remaining actions before formal status flip

1. Record final promote/defer decision in the GA go/no-go record.
2. Update repository status text from `Incubator` to promoted state if approved.
3. Keep governance indexes (`docs/decisions.md`, release notes/checklist) aligned with that decision.
