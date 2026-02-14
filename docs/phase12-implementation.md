# Phase 12 Implementation Progress

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13

## Completed in this pass

1. LSP client foundation
   - Added platform contract: `platform/include/ILspClientService.h`
   - Added Linux implementation: `platform/linux/LinuxLspClientService.h`, `platform/linux/LinuxLspClientService.cpp`
   - Added smoke coverage: `tests/platform/lsp_client_foundation_smoke_test.cpp`
   - Wired into build and tests in `CMakeLists.txt`

2. Syntax-aware autocomplete assists
   - Added paired-delimiter auto-close in Qt editor (`()`, `[]`, `{}`, `""`, `''`) in `ui/qt/MainWindow.cpp`
   - Added persisted preference toggle `autoCloseDelimiters` in `editor-settings.json` handling
   - Extended preferences UI with "Auto-close paired delimiters"

3. Color-coded syntax improvements
   - Extended lexer color styling for HTML/XML and Markdown in `ui/qt/MainWindow.cpp`
   - Existing C++/Python/Bash styling retained

4. Crash-recovery journal and restore UX
   - Added periodic crash journal autosave and startup recovery prompt in `ui/qt/MainWindow.cpp`
   - Added journal persistence settings in `editor-settings.json`
   - Added documentation: `docs/crash-recovery-journal.md`

5. Extension performance budgets and startup guardrails
   - Added startup-time and per-extension budget checks in `ui/qt/MainWindow.cpp`
   - Added diagnostics emission for guardrail violations
   - Added documentation: `docs/extension-performance-guardrails.md`

6. Distro validation expansion plan
   - Added cross-distro validation matrix and rollout criteria in `docs/distro-validation-plan.md`

## Phase 12 status

All planned Phase 12 TODO deliverables are now implemented.
