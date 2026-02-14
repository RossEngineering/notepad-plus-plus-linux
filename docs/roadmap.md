# Roadmap

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This timeline reflects project direction as of February 14, 2026.
RC3, RC3a, and RC3b are complete and published. The project now focuses on final
1.0.0 promotion.

## Milestones

| Milestone | Target Range | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | Completed | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | Completed | Phase 8 complete (Linux-first docs, migration issue workflows, dashboard). | Done |
| M2: Beta 1 (`v0.8.0-beta.1`) | Completed | Phase 9 complete (automatic language detection + stable auto-highlighting behavior). | Done |
| M3: Beta 2 (`v0.9.0-beta.2`) | Completed | Phases 10-12 complete (skinning, extension platform, hardening/language intelligence). | Done |
| M4: RC1 (`v0.9.3-rc.1`) | Completed | Correctness/stability pass, distro matrix baseline, extension permission hardening. | Done |
| M5: RC2 (`v0.9.6-rc.2`) | Completed | Deeper language-intelligence wiring, compatibility expansion, CI-enforced performance budgets. | Done |
| M6: RC3 (`v0.9.9-rc.3`) | Completed | Feature freeze, full regression sweep, release engineering dry-run, blocker closure. | Done |
| M6a: RC3a (`v0.9.9-rc.3a`) | Completed | Post-freeze maintenance respin for formatter UX and targeted low-risk fixes. | Done |
| M6b: RC3b (`v0.9.9-rc.3b`) | Completed | Post-freeze maintenance respin for language auto-detect default/toggle and issue-link correctness fixes. | Done |
| M7: General Availability (`v1.0.0`) | Current | Final go/no-go and production release. | In Progress |

## RC train structure

1. RC1 train: `v0.9.1` through `v0.9.3-rc.1`
2. RC2 train: `v0.9.4` through `v0.9.6-rc.2`
3. RC3 train: `v0.9.7` through `v0.9.9-rc.3`
4. RC3a maintenance respin: `v0.9.9-rc.3a` (exception lane)
5. RC3b maintenance respin: `v0.9.9-rc.3b` (exception lane)

## Proposed scope by RC

### RC1 (`v0.9.3-rc.1`) - stabilization baseline

1. Resolve highest-impact beta defects (crash, recovery, incorrect editor behavior, extension permission edge cases).
2. Complete first required distro validation evidence set:
   - Arch Linux derivatives baseline
   - Ubuntu LTS baseline
   - Fedora stable baseline
3. Ensure color-coded syntax behavior is consistent across targeted languages/themes.
4. Produce a strict RC1 release checklist and pass all required CI/sanitizer lanes.

### RC2 (`v0.9.6-rc.2`) - integration hardening

1. Move LSP from foundation to baseline user value (diagnostics, hover, go-to-definition minimum viable path).
2. Expand VS Code language-asset compatibility coverage and regression corpus.
3. Enforce startup and extension performance budgets in CI (not diagnostics-only).
4. Validate packaging/documentation quality for all required distros in the matrix.

### RC3 (`v0.9.9-rc.3`) - release readiness

1. Enter feature freeze; accept only bug fixes, docs, release engineering, and reliability changes.
   - Freeze policy: `docs/feature-freeze-rc3.md`
2. Deliver consumer-friendly install UX across target distros:
   - package install/uninstall polish
   - desktop/launcher/dock registration
   - file-handler and default app integration
3. Execute full cross-feature regression suite (editor core, language features, skinning, extension lifecycle, crash recovery).
4. Run release dry-run (artifacts, checksums, optional signing, rollback plan) and verify reproducibility controls.
5. Reach zero open release blockers for go/no-go review.

### RC3a (`v0.9.9-rc.3a`) - maintenance respin

1. Keep freeze discipline:
   - only user-visible correctness/polish and release documentation alignment
   - no broad net-new platform surface
2. Deliver language-aware formatting baseline behavior for core workflows.
3. Allow extension-declared formatter providers with explicit permission gating.
4. Re-run targeted regression and CI quality gates for formatter and editor safety.
5. Publish `v0.9.9-rc.3a` and carry forward as the final pre-GA baseline.

### RC3b (`v0.9.9-rc.3b`) - maintenance respin

1. Keep freeze discipline:
   - only user-visible correctness/polish and release-link consistency fixes
   - no broad net-new platform surface
2. Enable language auto-detection by default with explicit opt-out in preferences.
3. Preserve manual language lock/manual auto-detect behavior.
4. Re-run targeted CI gates (Linux CMake Build, Linux Desktop Integration, Win32 boundary guard).
5. Publish `v0.9.9-rc.3b` and carry forward as the final pre-GA baseline.

## 1.0.0 promotion gate

1. Zero open P0 defects.
2. No unresolved data-loss or crash-recovery regressions.
3. Required Linux CI lanes green for 7 consecutive days before final tag.
4. `v1.0.0` release notes and migration guidance finalized and reviewed.
5. Incubator promotion decision recorded (promote/defer) with README/governance docs aligned.

## GA execution artifacts

1. GA blocker tracker: `docs/ga-blockers.md`
2. CI continuity evidence log: `docs/ga-ci-continuity-log-2026-02.md`
3. Final go/no-go review: `docs/v1.0.0-go-no-go-review-2026-02-14.md`
4. GA release checklist: `docs/releases/v1.0.0-checklist.md`

## Delivery cadence rule

1. Complete work by TODO section.
2. At the end of each completed TODO section, publish a tagged prerelease.
3. Update release checklist and release notes in the same section-close pass.

## Next planning outputs

1. `docs/releases/v0.9.3-rc.1-checklist.md`
2. `docs/releases/v0.9.6-rc.2-checklist.md`
3. `docs/releases/v0.9.9-rc.3-checklist.md`
4. `docs/releases/v0.9.9-rc.3a-checklist.md`
5. `docs/releases/v0.9.9-rc.3b-checklist.md`
6. `docs/releases/v1.0.0-checklist.md`
7. `docs/releases/v0.10.0-beta.1-checklist.md`
8. `docs/releases/v0.10.0-beta.2-checklist.md`
9. `docs/releases/v0.10.0-beta.3-checklist.md`
10. `docs/releases/v0.10.0-beta.4-checklist.md`
11. `docs/releases/v0.10.0-beta.5-checklist.md`
12. `docs/releases/v0.10.0-beta.6-checklist.md`
13. `docs/releases/v0.10.0-beta.7-checklist.md`
