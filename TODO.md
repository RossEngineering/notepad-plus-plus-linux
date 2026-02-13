# TODO - notepad-plus-plus-linux

This file now tracks active post-Phase-8 work.
Completed Phases 0-8 are archived in `docs/todo-archive-phases-0-8.md`.

## Release direction

- [ ] Complete Phase 9 and cut `v0.8.0-beta.1`.
- [ ] No further alpha tags after `v0.7.0-alpha.1`.

## Phase 9: Automatic language detection and highlighting

- [ ] Implement automatic language detection using extension, shebang, modelines, and content heuristics.
- [ ] Detect and classify Markdown/HTML/programming-language files on open.
- [ ] Auto-switch lexer and syntax highlighting when detection confidence is high.
- [ ] Re-run detection on rename/save-as and provide manual override lock.
- [ ] Add detection/lexer regression corpus and accuracy tracking in tests.
- [ ] Define Beta 1 acceptance threshold for language detection accuracy and false-positive rate.

## Beta 1 release gate

- [ ] Freeze Beta 1 scope to Phase 9 deliverables.
- [ ] Run full Linux CI + sanitizer lane green for Beta 1 candidate tag.
- [ ] Validate release artifacts and checksums for Beta 1.
- [ ] Publish `v0.8.0-beta.1` notes and milestone summary.

## Post-Beta backlog: Phase 10 UI skinning support

- [ ] Define skin/theme format covering app chrome + editor + dialogs.
- [ ] Ship first-party skin set (light, dark, and high-contrast).
- [ ] Add runtime skin switcher and persistent per-user selection.
- [ ] Ensure full visual consistency across tabs, menus, status bar, and dialogs.
- [ ] Add accessibility checks for contrast/focus visibility in CI.

## Post-Beta backlog: Phase 11 extensions and VS Code compatibility strategy

- [ ] Define Linux extension API v1 boundaries and security model.
- [ ] Build extension lifecycle support (discover/install/enable/disable/remove).
- [ ] Implement extension permission prompts for filesystem/network/process access.
- [ ] Write ADR comparing VS Code compatibility options (full API parity vs targeted compatibility).
- [ ] Implement targeted compatibility path for VS Code language assets (TextMate grammars + language configs).
- [ ] Validate compatibility with at least 3 popular VS Code language extensions.

## Post-Beta backlog: Phase 12 suggested hardening work (recommended)

- [ ] Add LSP client foundation for richer language intelligence.
- [ ] Improve crash-recovery journal and restore UX.
- [ ] Add extension performance budgets and startup impact guardrails.
- [ ] Expand distro validation plan beyond Manjaro after baseline remains stable.

## Immediate next actions

- [ ] Create language-detection fixture corpus for Markdown/HTML/code.
- [ ] Implement confidence-scored auto-lexer switching with manual override lock.
- [ ] Define Beta 1 detection acceptance metrics and test report format.
- [ ] Draft `v0.8.0-beta.1` release checklist and notes skeleton.
