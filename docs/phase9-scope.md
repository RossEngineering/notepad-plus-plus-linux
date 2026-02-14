# Phase 9+ Scope Brainstorm

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Date: 2026-02-13

This document proposes post-Phase-8 priorities aligned with `TODO.md` and `docs/roadmap.md`.

## Goal

Phase 9 is complete and shipped as `v0.8.0-beta.1`.
Next goal is Beta 2, defined as completion of Phases 10-12.

## Workstream 1: Automatic language detection and highlighting (Phase 9)

1. Detect language via extension, shebang, modelines, and content heuristics.
2. Classify Markdown/HTML/programming languages accurately on open and rename/save-as.
3. Auto-select lexer with confidence gates and manual override lock.
4. Track accuracy and false-positive rate on regression corpus.
5. Use Phase 9 completion as the release gate for `v0.8.0-beta.1`.

## Workstream 2: UI skinning support (Phase 10)

1. Define a stable skin format for shell/editor/dialog theming.
2. Ship light/dark/high-contrast first-party skins.
3. Provide runtime skin switcher with persisted selection.
4. Add accessibility validation (contrast/focus visibility).

## Workstream 3: Extension platform and VS Code compatibility strategy (Phase 11)

1. Define extension API v1 and permission model.
2. Implement extension lifecycle manager (install/enable/disable/remove).
3. Publish ADR for VS Code compatibility scope and non-goals.
4. Build targeted compatibility path for VS Code language assets (TextMate + language configs).

## Workstream 4: Hardening recommendations (Phase 12)

1. Add LSP client foundation.
2. Add syntax-aware autocomplete assists (for example HTML tag auto-close and paired delimiters).
3. Improve crash-recovery journal and restore UX.
4. Add extension performance budget checks.
5. Plan distro expansion once the Arch-derivatives baseline (including Manjaro) is stable.

## Release strategy note

- No further alpha releases are planned after `v0.7.0-alpha.1`.
- `v0.8.0-beta.1` is complete.
- Next planned release target is `v0.9.0-beta.2` after Phases 10-12.
