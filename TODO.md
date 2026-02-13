# TODO - notepad-plus-plus-linux

This file tracks the Linux-native migration and modernization work for this fork.

## Phase 0: Baseline and audit

- [x] Tag current fork baseline commit.
- [x] Document current build status on Manjaro (what compiles, what fails, error logs).
- [x] Inventory Win32 API usage in `PowerEditor/src`.
- [x] Identify modules that are already cross-platform (or mostly portable).
- [x] Define compatibility target: feature parity scope for first Linux release.

## Phase 1: Architecture boundaries

- [x] Create architecture doc under `docs/architecture.md`.
- [x] Define `core` boundary (buffer, undo/redo, search, encoding, session state).
- [x] Define `platform` boundary (filesystem, clipboard, process, settings, dialogs).
- [x] Define `ui` boundary and message/event flow.
- [x] Add coding rules that block new direct Win32 calls outside platform layer.

## Phase 2: Build system modernization

- [x] Introduce top-level CMake configuration for Linux builds.
- [x] Build `scintilla` and `lexilla` as Linux targets through CMake.
- [x] Establish Debug/Release presets for local development.
- [x] Add CI job for Linux build and basic tests.
- [x] Keep existing Windows build path functional during transition.

## Phase 3: Platform abstraction layer

- [x] Add interface layer for OS services (path/file, clipboard, timers, subprocess).
- [x] Port file and path handling to UTF-8-first Linux behavior.
- [x] Implement XDG config/data/cache directory handling.
- [x] Implement Linux crash logging and diagnostics path.
- [x] Replace direct Win32 calls incrementally behind interfaces.

## Phase 4: Linux-native UI

- [x] Select initial UI backend (Qt recommended for first stable path).
- [x] Create Linux application shell (main window, tabs, menus, status bar).
- [x] Integrate Scintilla widget for Linux frontend.
- [x] Port essential dialogs (find/replace, preferences, go to line).
- [x] Implement keyboard shortcuts with Linux-friendly defaults and override support.

## Phase 5: Feature parity (MVP)

- [x] File open/save/reload with encoding and EOL handling.
- [x] Multi-tab editing and session restore.
- [x] Search/replace (document and multi-file basic mode).
- [x] Syntax highlighting and theme loading.
- [ ] External tools / run command support.
- [ ] Plugin story decision: compatibility shim or new Linux plugin API.

## Phase 6: Packaging and distribution (Manjaro first)

- [ ] Add `PKGBUILD` for local package builds.
- [ ] Define package split strategy (runtime, debug symbols, optional plugins).
- [ ] Add desktop entry, icon assets, and MIME associations.
- [ ] Add reproducible release build instructions.
- [ ] Publish signed release artifacts and checksums.

## Phase 7: Quality and performance

- [ ] Add unit tests for core text operations and undo/redo behavior.
- [ ] Add regression tests for encoding conversion and large-file handling.
- [ ] Add syntax highlighting smoke tests across representative languages.
- [ ] Benchmark startup time and typing latency against baseline.
- [ ] Add memory/leak checks in CI for Linux builds.

## Phase 8: Documentation and developer UX

- [ ] Rewrite `BUILD.md` with Linux-native instructions first.
- [ ] Add `docs/dev-setup-manjaro.md` for one-command setup.
- [ ] Add `docs/roadmap.md` with milestone timeline.
- [ ] Add contribution labels and issue templates for migration work.
- [ ] Add migration status dashboard to README.

## Immediate next actions

- [x] Create `docs/architecture.md` with first module map.
- [x] Run Win32 API usage report and commit results to `docs/audit-win32.md`.
- [ ] Prototype Linux CMake build for `scintilla` + `lexilla`.
- [x] Decide UI backend and record rationale in `docs/ui-decision.md`.
