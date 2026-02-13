# TODO - notepad-plus-plus-linux

This file now tracks active post-Phase-8 work.
Completed Phases 0-8 are archived in `docs/todo-archive-phases-0-8.md`.
Completed Beta 1 and Beta 2 details are archived in `docs/todo-archive-beta-1-2.md`.

## Release direction

- [x] Complete Phase 9 and cut `v0.8.0-beta.1`.
- [x] No further alpha tags after `v0.7.0-alpha.1`.
- [x] Complete Phases 10-12 and cut `v0.9.0-beta.2`.
- [x] Complete RC1 scope and cut `v0.9.3-rc.1`.
- [ ] Complete RC2 scope and cut `v0.9.6-rc.2`.
- [ ] Complete RC3 scope and cut `v0.9.9-rc.3`.
- [ ] Promote `v1.0.0` after RC3 validation.

## Release track from Beta 2 to 1.0.0

### RC1 program (`v0.9.1` to `v0.9.3-rc.1`)

- [x] Close top P0/P1 correctness and crash issues discovered during beta usage.
- [x] Ship end-to-end color-coded syntax style consistency across supported lexers.
- [x] Complete first-pass distro matrix validation (Arch derivatives, Ubuntu LTS, Fedora stable).
- [x] Harden extension permissions UX and denial-path behavior.
- [x] Add in-app Help menu entries for docs/wiki plus bug and feature submission links.
- [x] Cut and validate `v0.9.3-rc.1`.

### RC2 program (`v0.9.4` to `v0.9.6-rc.2`)

- [ ] Complete LSP wiring from foundation into core editor UX (diagnostics, hover, go-to-definition baseline).
- [ ] Add extension compatibility pass for additional VS Code language assets and edge-case grammars.
- [ ] Tighten startup/performance budgets with CI-enforced thresholds.
- [ ] Finalize packaging/install docs for Arch derivatives and additional validated distros.
- [ ] Cut and validate `v0.9.6-rc.2`.

### RC3 program (`v0.9.7` to `v0.9.9-rc.3`)

- [ ] Enter feature freeze (bug fixes, docs, packaging, and release reliability only).
- [ ] Run full regression on editor, skinning, extension lifecycle, crash recovery, and language workflows.
- [ ] Complete release engineering dry-run (artifacts, checksums, signing, rollback plan).
- [ ] Resolve all open release blockers and cut `v0.9.9-rc.3`.
- [ ] Perform go/no-go review for `v1.0.0`.

### 1.0.0 promotion gate

- [ ] Zero open P0 defects.
- [ ] No unresolved data-loss/crash-recovery regressions.
- [ ] Linux required CI lanes green for 7 consecutive days before tag.
- [ ] Publish `v1.0.0` release notes and migration guidance.

## Immediate next actions

- [x] Create `v0.9.3-rc.1` checklist and notes stub in `docs/releases/`.
- [x] Build and prioritize RC1 blocker list from latest beta feedback.
- [x] Define RC1 distro validation evidence format and publish first report.
- [x] Add `docs/releases/v1.0.0-checklist.md` draft with final promotion gates.
