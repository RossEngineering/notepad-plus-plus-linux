# TODO Archive - Beta 1 and Beta 2

Archived on 2026-02-13 from `TODO.md`.
This file preserves completed Beta 1 and Beta 2 execution detail.

## Phase 9: Automatic language detection and highlighting

- [x] Implement automatic language detection using extension, shebang, modelines, and content heuristics.
- [x] Detect and classify Markdown/HTML/programming-language files on open.
- [x] Auto-switch lexer and syntax highlighting when detection confidence is high.
- [x] Re-run detection on rename/save-as and provide manual override lock.
- [x] Add detection/lexer regression corpus and accuracy tracking in tests.
- [x] Define Beta 1 acceptance threshold for language detection accuracy and false-positive rate.

## Beta 1 release gate

- [x] Freeze Beta 1 scope to Phase 9 deliverables.
- [x] Run full Linux CI + sanitizer lane green for Beta 1 candidate tag.
- [x] Validate release artifacts and checksums for Beta 1.
- [x] Publish `v0.8.0-beta.1` notes and milestone summary.

## Beta 2 release gate

- [x] Freeze Beta 2 scope to Phase 10-12 deliverables.
- [x] Run full Linux CI + sanitizer lane green for Beta 2 candidate tag.
- [x] Validate release artifacts and checksums for Beta 2.
- [x] Publish `v0.9.0-beta.2` notes and milestone summary.

## Beta 2 scope: Phase 10 UI skinning support

- [x] Define skin/theme format covering app chrome + editor + dialogs.
- [x] Ship first-party skin set (light, dark, and high-contrast).
- [x] Add runtime skin switcher and persistent per-user selection.
- [x] Ensure full visual consistency across tabs, menus, status bar, and dialogs.
- [x] Add accessibility checks for contrast/focus visibility in CI.

## Beta 2 scope: Phase 11 extensions and VS Code compatibility strategy

- [x] Define Linux extension API v1 boundaries and security model.
- [x] Build extension lifecycle support (discover/install/enable/disable/remove).
- [x] Implement extension permission prompts for filesystem/network/process access.
- [x] Write ADR comparing VS Code compatibility options (full API parity vs targeted compatibility).
- [x] Implement targeted compatibility path for VS Code language assets (TextMate grammars + language configs).
- [x] Validate compatibility with at least 3 popular VS Code language extensions.

## Beta 2 scope: Phase 12 hardening and language intelligence

- [x] Add LSP client foundation for richer language intelligence.
- [x] Add syntax-aware autocomplete assists (for example HTML tag auto-close and paired delimiters).
- [x] Improve crash-recovery journal and restore UX.
- [x] Add extension performance budgets and startup impact guardrails.
- [x] Expand distro validation plan beyond Arch Linux derivatives after baseline remains stable.

## Immediate next actions at Beta 1 stage (historical)

- [x] Create language-detection fixture corpus for Markdown/HTML/code.
- [x] Implement confidence-scored auto-lexer switching with manual override lock.
- [x] Define Beta 1 detection acceptance metrics and test report format.
- [x] Draft `v0.8.0-beta.1` release checklist and notes skeleton.
