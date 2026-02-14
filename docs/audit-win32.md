# Win32 Audit

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13
Scope: `PowerEditor/src`

## Goal

Establish a concrete baseline of Windows API coupling to drive Linux migration priority.

## Method

Quick static scan was run over `PowerEditor/src` with `rg` for:

- Includes: `#include <windows.h>`, `#include <Windows.h>`
- Win32 types/messages/functions: `HWND`, `HMENU`, `HINSTANCE`, `LPARAM`, `WPARAM`, `LRESULT`, `SendMessage`, `CreateWindow`, `RegisterClass`

This is a prioritization scan, not a semantic proof.

## Summary metrics

- Files including `windows.h`: `96`
- Files containing Win32 core types/messages: `152`

Interpretation:

- Win32 dependency is broad, not isolated to a single shell file.
- Migration should begin with boundary extraction, not direct one-file rewrite.

## Hotspot files (by match density)

Top files by matched Win32 tokens:

1. `PowerEditor/src/WinControls/Preference/preferenceDlg.cpp` (387)
2. `PowerEditor/src/ScintillaComponent/FindReplaceDlg.cpp` (213)
3. `PowerEditor/src/ScintillaComponent/ScintillaEditView.cpp` (174)
4. `PowerEditor/src/NppDarkMode.cpp` (174)
5. `PowerEditor/src/Notepad_plus.cpp` (156)
6. `PowerEditor/src/WinControls/Grid/BabyGrid.cpp` (121)
7. `PowerEditor/src/NppBigSwitch.cpp` (108)
8. `PowerEditor/src/ScintillaComponent/UserDefineDialog.cpp` (98)
9. `PowerEditor/src/WinControls/DockingWnd/DockingCont.cpp` (64)
10. `PowerEditor/src/WinControls/TabBar/TabBar.cpp` (61)

## Key dependency clusters

## 1) Application shell and message loop

Files:

- `PowerEditor/src/winmain.cpp`
- `PowerEditor/src/Notepad_plus_Window.cpp`
- `PowerEditor/src/Notepad_plus.cpp`
- `PowerEditor/src/NppBigSwitch.cpp`

Traits:

- Direct WinMain startup and window class registration.
- Heavy `SendMessage`-based internal command routing.

Risk:

- High. This is central control flow and must be restructured early.

## 2) UI controls and dialogs

Files:

- `PowerEditor/src/WinControls/**`
- `PowerEditor/src/ScintillaComponent/FindReplaceDlg.*`
- `PowerEditor/src/ScintillaComponent/UserDefineDialog.*`

Traits:

- Win32 dialog procedures and message handling.
- Control-specific behavior tied to HWND and resource scripts.

Risk:

- High volume but parallelizable once a Linux UI shell exists.

## 3) Theming and dark mode

Files:

- `PowerEditor/src/NppDarkMode.cpp`
- `PowerEditor/src/NppDarkMode.h`
- `PowerEditor/src/DarkMode/*`

Traits:

- Deep coupling to Windows theming APIs and subclassing.

Risk:

- High complexity, medium immediate priority. Reimplement Linux-native theming instead of porting behavior line-by-line.

## 4) DPI and shell integration

Files:

- `PowerEditor/src/dpiManagerV2.cpp`
- `PowerEditor/src/MISC/Exception/Win32Exception.cpp`

Traits:

- Windows DPI calls and Windows-specific exception handling.

Risk:

- Medium. Replace with cross-platform equivalents via abstraction.

## Recommendations

1. Freeze new direct Win32 calls in migration-targeted paths.
2. Introduce `platform` interfaces and migrate call sites incrementally.
3. Prioritize extracting behavior from `Notepad_plus.cpp` and command dispatch logic first.
4. Rebuild dialogs in chosen Linux UI toolkit rather than emulating Win32 dialog procs.
5. Defer dark-mode parity until core editing and command workflows are stable.

## Immediate follow-up tasks

- Produce a categorized API map (`windowing`, `clipboard`, `filesystem`, `process`, `theme`, `dpi`).
- Tag each hotspot with migration strategy:
  - Wrap
  - Replace
  - Delete
- Start with one vertical slice:
  - Linux shell startup
  - One tab with Scintilla
  - Open/save
  - Find in document
