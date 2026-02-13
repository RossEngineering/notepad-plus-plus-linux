# TODO - notepad-plus-plus-linux

This file now tracks active post-Phase-8 work.
Completed Phases 0-8 are archived in `docs/todo-archive-phases-0-8.md`.

## Phase 9: Automatic language detection and highlighting

- [ ] Implement automatic language detection using extension, shebang, modelines, and content heuristics.
- [ ] Detect and classify Markdown/HTML/programming-language files on open.
- [ ] Auto-switch lexer and syntax highlighting when detection confidence is high.
- [ ] Re-run detection on rename/save-as and provide manual override lock.
- [ ] Add detection/lexer regression corpus and accuracy tracking in tests.

## Phase 10: UI skinning support

- [ ] Define skin/theme format covering app chrome + editor + dialogs.
- [ ] Ship first-party skin set (light, dark, and high-contrast).
- [ ] Add runtime skin switcher and persistent per-user selection.
- [ ] Ensure full visual consistency across tabs, menus, status bar, and dialogs.
- [ ] Add accessibility checks for contrast/focus visibility in CI.

## Phase 11: Extensions and VS Code compatibility strategy

- [ ] Define Linux extension API v1 boundaries and security model.
- [ ] Build extension lifecycle support (discover/install/enable/disable/remove).
- [ ] Implement extension permission prompts for filesystem/network/process access.
- [ ] Write ADR comparing VS Code compatibility options (full API parity vs targeted compatibility).
- [ ] Implement targeted compatibility path for VS Code language assets (TextMate grammars + language configs).
- [ ] Validate compatibility with at least 3 popular VS Code language extensions.

## Phase 12: Suggested hardening work (recommended)

- [ ] Add LSP client foundation for richer language intelligence.
- [ ] Improve crash-recovery journal and restore UX.
- [ ] Add extension performance budgets and startup impact guardrails.
- [ ] Expand distro validation plan beyond Manjaro after baseline remains stable.

## Immediate next actions

- [ ] Draft VS Code compatibility ADR with explicit non-goals.
- [ ] Create language-detection fixture corpus for Markdown/HTML/code.
- [ ] Draft skin schema and provide two sample skin files.
- [ ] Define extension permission categories and default-deny policy.
