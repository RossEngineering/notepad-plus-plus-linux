# TODO - notepad-plus-plus-linux

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This file now tracks active post-Phase-8 work.
Completed Phases 0-8 are archived in `docs/todo-archive-phases-0-8.md`.
Completed Beta 1 and Beta 2 details are archived in `docs/todo-archive-beta-1-2.md`.

## Release direction

- [x] Complete Phase 9 and cut `v0.8.0-beta.1`.
- [x] No further alpha tags after `v0.7.0-alpha.1`.
- [x] Complete Phases 10-12 and cut `v0.9.0-beta.2`.
- [x] Complete RC1 scope and cut `v0.9.3-rc.1`.
- [x] Complete RC2 scope and cut `v0.9.6-rc.2`.
- [x] Complete RC3 scope and cut `v0.9.9-rc.3`.
- [x] Cut RC3a maintenance release `v0.9.9-rc.3a`.
- [x] Cut RC3b maintenance release `v0.9.9-rc.3b`.
- [ ] Complete GA scope and cut `v1.0.0`.
- [ ] Promote `v1.0.0` after RC3 maintenance validation.

## Release track from Beta 2 to 1.0.0

### RC1 program (`v0.9.1` to `v0.9.3-rc.1`)

- [x] Close top P0/P1 correctness and crash issues discovered during beta usage.
- [x] Ship end-to-end color-coded syntax style consistency across supported lexers.
- [x] Complete first-pass distro matrix validation (Arch derivatives, Ubuntu LTS, Fedora stable).
- [x] Harden extension permissions UX and denial-path behavior.
- [x] Add in-app Help menu entries for docs/wiki plus bug and feature submission links.
- [x] Cut and validate `v0.9.3-rc.1`.

### RC2 program (`v0.9.4` to `v0.9.6-rc.2`)

- [x] Add an About dialog under Help with version/build metadata and support links.
- [x] Complete LSP wiring from foundation into core editor UX (diagnostics, hover, go-to-definition baseline).
- [x] Add extension compatibility pass for additional VS Code language assets and edge-case grammars.
- [x] Tighten startup/performance budgets with CI-enforced thresholds.
- [x] Finalize packaging/install docs for Arch derivatives and additional validated distros.
- [x] Cut and validate `v0.9.6-rc.2`.

### RC3 program (`v0.9.7` to `v0.9.9-rc.3`)

- [x] Enter feature freeze (bug fixes, docs, packaging, and release reliability only).
- [x] Deliver consumer-friendly Linux install flow (package install, desktop registration, launcher/dock visibility, clean uninstall behavior).
- [x] Validate file-handler integration end-to-end (MIME/app defaults, open-with behavior, double-click launch) across Arch derivatives, Ubuntu LTS, and Fedora stable.
- [x] Run full regression on editor, skinning, extension lifecycle, crash recovery, and language workflows.
- [x] Complete release engineering dry-run (artifacts, checksums, signing, rollback plan).
- [x] Resolve all open release blockers and cut `v0.9.9-rc.3`.
- [x] Perform go/no-go review for `v1.0.0`.

### RC3a maintenance program (`v0.9.9-rc.3a`)

- [x] Add language-aware `Format Document` baseline behavior (Python indentation-sensitive normalization, line-ending preservation, and safe no-op fallback).
- [x] Add extension-declared formatter contributions so executable extensions can act as language formatters.
- [x] Gate formatter execution behind extension permissions and deterministic failure handling.
- [x] Validate formatter paths in debug/release CI and regression runs.
- [x] Cut and validate `v0.9.9-rc.3a`.

### RC3b maintenance program (`v0.9.9-rc.3b`)

- [x] Enable language auto-detection by default for out-of-box behavior.
- [x] Add Preferences toggle to disable automatic language detection.
- [x] Preserve manual language lock and manual auto-detect behavior.
- [x] Align in-app bug reporting links to `bug_report.yml`.
- [x] Cut and validate `v0.9.9-rc.3b`.

### GA program (`v1.0.0`)

- [x] Lock final RC baseline at `v0.9.9-rc.3b`.
- [ ] Close all `P0` GA blockers in `docs/ga-blockers.md`.
- [ ] Keep the 7-day continuity record current in `docs/ga-ci-continuity-log-2026-02.md`.
- [ ] Finalize `docs/releases/v1.0.0.md` migration guidance and known limitations.
- [ ] Record explicit incubator promote/defer decision and sync repository status text.
- [ ] Cut and validate `v1.0.0`.

### External tester intake (GA window)

- [x] Route community bug reports to `bug_report.yml`.
- [ ] Triage new community bug reports daily during GA window.
- [ ] Maintain a rolling distro/kernel/DE coverage snapshot from incoming tester issues (`docs/tester-coverage-snapshot-2026-02.md`).
- [ ] Convert confirmed crash/data-loss/installer regressions into `docs/ga-blockers.md` entries.
- [ ] Close or defer non-GA feedback with clear milestone tagging (`v1.0.0` vs post-GA).

### 1.0.0 promotion gate

- [ ] Zero open P0 defects.
- [ ] No unresolved data-loss/crash-recovery regressions.
- [ ] Linux required CI lanes green for 7 consecutive days before tag.
- [ ] Publish `v1.0.0` release notes and migration guidance.

### Incubator promotion gate

- [x] Re-run incubator promotion validation at RC3 completion.
- [ ] Record explicit promote/defer decision for incubator exit during `v1.0.0` go/no-go.
- [ ] Update `README.md` repository status and linked governance docs to match incubator decision.

## Immediate next actions

- [x] Add consumer local installer/uninstaller flow with desktop, MIME, and icon-cache registration support.
- [x] Add automated desktop/file-handler integration validation lane for Arch derivatives, Ubuntu LTS, and Fedora stable.
- [x] Create and prioritize RC3 blocker list from RC2 exit signals.
- [x] Create `v0.9.9-rc.3` release checklist and notes stub in `docs/releases/`.
- [x] Create `v0.9.9-rc.3a` release checklist and notes stub in `docs/releases/`.
- [x] Create `v0.9.9-rc.3b` release checklist and notes stub in `docs/releases/`.
- [x] Create GA blocker tracker and continuity log scaffolding (`docs/ga-blockers.md`, `docs/ga-ci-continuity-log-2026-02.md`).
- [x] Add one-command GA CI continuity log updater (`scripts/ga/update_ci_continuity_log.sh`).
- [x] Complete Milestone A Win32 cleanup for Linux build path (remove legacy include bleed + enforce strict Linux-source Win32 CI guard).
- [x] Add tester quickstart and Linux-focused bug intake template for community validation.
- [x] Add GA issue triage playbook and tester coverage snapshot scaffolding.
- [ ] Update GA continuity log daily until 7/7 pass window is closed.

## Post-GA feature wishlist (ongoing dev branch)

### Editor UX and workflows

- [ ] Add split editor view (vertical and horizontal) with synchronized tab/session behavior.
- [ ] Add minimap toggle and viewport highlight for large files.
- [ ] Add multi-cursor editing baseline (add cursor, next match, all matches).
- [x] Add command palette for discoverable quick actions.
- [ ] Add configurable autosave modes (focus lost, interval, on build/run).
- [ ] Add persistent workspace/session restore options with per-project settings.

### Formatting, language intelligence, and coding tools

- [ ] Add formatter selection per language (default + override).
- [ ] Add format-on-save toggle globally and per language.
- [ ] Add in-editor diagnostics panel with quick navigation and filtering.
- [ ] Add symbol outline and document symbol search.
- [ ] Add rename symbol baseline through LSP where supported.
- [ ] Add code actions baseline (quick fixes where language server supports it).

### Extensions ecosystem

- [ ] Publish extension authoring guide for formatter and language contributions.
- [ ] Add in-app extension marketplace placeholder view (local index first).
- [ ] Add extension update notifications and one-click update flow.
- [ ] Add per-extension resource usage and startup impact visibility.
- [ ] Add safe mode launch option to disable all extensions for troubleshooting.

### Linux integration and distribution

- [ ] Add native package outputs for Arch (`.pkg.tar.zst`), Debian/Ubuntu (`.deb`), and Fedora (`.rpm`).
- [ ] Add optional auto-update channel model (stable/candidate) for packaged installs.
- [ ] Add desktop action entries (new file, open recent) in `.desktop` integration.
- [ ] Add portal-aware open/save handling improvements for sandboxed desktop environments.
- [ ] Add Wayland-focused QA pass and issue tracker labels for Wayland-specific regressions.

### Reliability, performance, and observability

- [ ] Add startup trace capture mode for profiling cold-start regressions.
- [ ] Expand performance CI budgets to include large-file open and search workloads.
- [ ] Add crash report bundle generation (local file) for easier issue attachments.
- [ ] Add regression fixtures for mixed-encoding and very-large-file editing scenarios.
- [ ] Add deterministic config migration tests for upgrades between minor versions.

### Adoption and product polish

- [ ] Add first-run onboarding page with key settings and keyboard shortcut tips.
- [ ] Add import wizard for relevant settings from popular Linux editors (opt-in).
- [ ] Add curated default skin and icon variants tuned for common desktop themes.
- [ ] Add "what's new" dialog for release-to-release user-facing changes.
- [ ] Build a public "top requested features" board sourced from GitHub issues/discussions.
