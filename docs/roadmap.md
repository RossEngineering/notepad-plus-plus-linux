# Roadmap

This timeline reflects project direction as of February 14, 2026.
RC2 is complete and published. The project now moves through RC3 and then 1.0.0.

## Milestones

| Milestone | Target Range | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | Completed | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | Completed | Phase 8 complete (Linux-first docs, migration issue workflows, dashboard). | Done |
| M2: Beta 1 (`v0.8.0-beta.1`) | Completed | Phase 9 complete (automatic language detection + stable auto-highlighting behavior). | Done |
| M3: Beta 2 (`v0.9.0-beta.2`) | Completed | Phases 10-12 complete (skinning, extension platform, hardening/language intelligence). | Done |
| M4: RC1 (`v0.9.3-rc.1`) | Completed | Correctness/stability pass, distro matrix baseline, extension permission hardening. | Done |
| M5: RC2 (`v0.9.6-rc.2`) | Completed | Deeper language-intelligence wiring, compatibility expansion, CI-enforced performance budgets. | Done |
| M6: RC3 (`v0.9.9-rc.3`) | Next | Feature freeze, full regression sweep, release engineering dry-run, blocker closure. | Planned |
| M7: General Availability (`v1.0.0`) | After satisfactory RC3 | Final go/no-go and production release. | Planned |

## RC train structure

1. RC1 train: `v0.9.1` through `v0.9.3-rc.1`
2. RC2 train: `v0.9.4` through `v0.9.6-rc.2`
3. RC3 train: `v0.9.7` through `v0.9.9-rc.3`

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
2. Deliver consumer-friendly install UX across target distros:
   - package install/uninstall polish
   - desktop/launcher/dock registration
   - file-handler and default app integration
3. Execute full cross-feature regression suite (editor core, language features, skinning, extension lifecycle, crash recovery).
4. Run release dry-run (artifacts, checksums, optional signing, rollback plan) and verify reproducibility controls.
5. Reach zero open release blockers for go/no-go review.

## 1.0.0 promotion gate

1. Zero open P0 defects.
2. No unresolved data-loss or crash-recovery regressions.
3. Required Linux CI lanes green for 7 consecutive days before final tag.
4. `v1.0.0` release notes and migration guidance finalized and reviewed.

## Next planning outputs

1. `docs/releases/v0.9.3-rc.1-checklist.md`
2. `docs/releases/v0.9.6-rc.2-checklist.md`
3. `docs/releases/v0.9.9-rc.3-checklist.md`
4. `docs/releases/v1.0.0-checklist.md`
