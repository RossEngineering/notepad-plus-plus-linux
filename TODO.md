# TODO - notepad-plus-plus-linux

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This file now tracks GA closeout and post-GA execution.

Previous plan snapshots:
- `docs/todo-archive-pre-ga-2026-02-14.md`
- `docs/todo-archive-beta-1-2.md`
- `docs/todo-archive-phases-0-8.md`

## GA closeout (`v1.0.0`)

- [ ] Close all open `P0` entries in `docs/ga-blockers.md`.
- [ ] Keep the 7-day continuity record current in `docs/ga-ci-continuity-log-2026-02.md`.
- [ ] Triage new community issues daily and route confirmed GA-impacting defects into `docs/ga-blockers.md`.
- [ ] Finalize `docs/releases/v1.0.0.md` migration guidance and known limitations.
- [ ] Complete `docs/releases/v1.0.0-checklist.md` and cut `v1.0.0`.

## Post-GA operating cadence

- [ ] Keep `master` as the stable release line after `v1.0.0`.
- [ ] Continue feature delivery on `ga-dev-weekN` branches and merge in controlled batches.
- [ ] At the end of each completed section below, cut a tagged prerelease and publish notes/checklist updates.

## Post-GA roadmap streams

### Stream A - Editor UX and workflows (`v1.1.x` target)

- [ ] Add dockable panels layout presets (coding, writing, debugging).
- [ ] Add keyboard shortcut profile presets (Notepad++, VS Code-like, custom export/import).
- [ ] Add advanced search/replace workflows (saved searches, result groups, scope history).
- [ ] Add session templates for common project shapes.
- [ ] Add configurable status bar widgets (encoding, EOL, language, diagnostics count).

### Stream B - Language intelligence and formatting (`v1.2.x` target)

- [ ] Add formatter timeout/retry policy with visible per-language status.
- [ ] Add per-project language overrides and formatter settings file support.
- [ ] Add richer code actions UI with preview before apply.
- [ ] Add inline diagnostics quick-fix affordances.
- [ ] Expand built-in language defaults for Python, C/C++, JavaScript/TypeScript, JSON, YAML, and Markdown.

### Stream C - Linux integration and distribution (`v1.3.x` target)

- [ ] Add signed package publication workflow for `.deb`, `.rpm`, and `.pkg.tar.zst`.
- [ ] Add distro-specific install docs for Arch derivatives, Ubuntu LTS, Fedora stable, and openSUSE Tumbleweed.
- [ ] Add auto-update channel controls (stable/candidate/nightly) in Preferences.
- [ ] Add better portal-first behavior for Flatpak/Snap environments.
- [ ] Add file-association repair tool in-app for broken desktop integration.

### Stream D - Reliability, observability, and scale (`v1.4.x` target)

- [ ] Add startup/crash telemetry export bundle (local-only, user-triggered).
- [ ] Add large-file editing guardrails and progressive feature degradation for huge inputs.
- [ ] Add long-run soak tests in CI for memory/leak/regression detection.
- [ ] Add configuration schema versioning and forward/backward migration tests.
- [ ] Add deterministic extension-host recovery tests (crash/restart/replay scenarios).
