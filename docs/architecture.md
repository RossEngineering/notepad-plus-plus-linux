# Architecture Plan

Last updated: 2026-02-13

## Purpose

Define a Linux-native target architecture for this fork while preserving core Notepad++ behavior and performance.

This plan is incremental. It allows shipping usable Linux milestones without a full rewrite in one step.

## Current state summary

- `PowerEditor/src` is tightly coupled to Win32 message-loop and control APIs.
- Scintilla and Lexilla are already separate codebases and should remain external editor/language engines.
- Existing `PowerEditor/src/CMakeLists.txt` is a Windows-oriented script and not a Linux build entrypoint.
- `scintilla/qt` contains Qt widget support that can accelerate Linux UI integration.

## Target module boundaries

## `core` (platform-agnostic)

Responsibilities:

- Text/document model and buffer lifecycle.
- Undo/redo semantics.
- Search/replace engine behavior.
- Encoding and EOL normalization behavior.
- Session/project state model (not persistence backend).

Constraints:

- No direct OS APIs.
- No direct UI toolkit types.
- Pure C++ interfaces for I/O, clipboard, process launch, notifications.

## `platform` (OS services)

Responsibilities:

- Filesystem and path services (UTF-8 on Linux).
- Clipboard integration.
- Process execution and environment lookup.
- Settings persistence paths (XDG config/data/cache).
- Crash/log output paths and runtime diagnostics.

Initial shape:

- `platform/include` for interfaces.
- `platform/linux` for Linux implementations.
- Optional `platform/win32` later to preserve dual-platform development.

## `ui` (Linux application frontend)

Responsibilities:

- Main window, tabs, menus, dialogs, status bar.
- Input mapping (keyboard/mouse/IME).
- Theme coordination and editor chrome.
- Translation of UI events into `core` commands.

Preferred first backend:

- Qt 6, using Scintilla Qt widget path as integration base.

## Suggested near-term directory structure

```text
docs/
core/
platform/
  include/
  linux/
ui/
  qt/
compat/
  win32-shims/
```

This structure can be introduced gradually while existing `PowerEditor/src` continues to compile on Windows.

## Data and control flow

1. UI dispatches command (`open-file`, `find-next`, `set-encoding`) to application layer.
2. Application layer invokes `core` use-case functions.
3. `core` uses abstract interfaces for OS-bound tasks.
4. `platform/linux` implements those interfaces.
5. UI observes state changes and repaints.

## Migration seams in current codebase

Primary extraction candidates:

- `ScintillaComponent/Buffer.*` and related buffer/session logic.
- `Utf8_16.*`, `EncodingMapper.*` for encoding services.
- Search flows in `ScintillaComponent/FindReplaceDlg.*` (logic should be split from dialog implementation).

Primary UI-heavy hotspots to isolate:

- `Notepad_plus.cpp`
- `NppBigSwitch.cpp`
- `NppCommands.cpp`
- `NppDarkMode.cpp`
- `WinControls/*`

## Milestone map

## M1: Core + platform interfaces

- Add interface headers and first Linux stubs.
- Move pure logic utilities behind interface use.
- Keep behavior parity validated by tests.

## M2: Linux shell + editor embedding

- Create Qt app shell with tab container.
- Embed Scintilla Qt editor widget.
- Support file open/save/new and basic editing.

## M3: Essential workflows

- Find/replace, preferences, encoding/EOL, session restore.
- Basic language highlighting/themes.
- Package as an Arch Linux derivative-friendly build artifact (Manjaro baseline).

## Non-goals for first Linux release

- Binary compatibility with existing Windows plugin ABI.
- Exact visual parity with Windows chrome behavior.
- Full 1:1 implementation of all legacy Windows dialogs.

## Guardrails

- New platform-dependent code must go behind `platform` interfaces.
- Avoid adding new Win32 dependencies in migrated code paths.
- Keep text engine performance checks in CI as modules are moved.
