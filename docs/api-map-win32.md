# Win32 API Map for Linux Migration

Last updated: 2026-02-13
Scope: `PowerEditor/src`

## Purpose

Map current Win32 API usage into migration categories and define Linux-side replacement direction.

This is a planning artifact for implementation sequencing, not a full call graph.

## Category summary

1. `windowing`: Highest impact; central message-loop and command routing dependency.
2. `filesystem`: Broad usage; should move behind UTF-8 path/file abstraction.
3. `clipboard`: Moderate usage; straightforward abstraction once UI backend is selected.
4. `process`: Moderate usage; needs clear split between open-url/open-path/spawn-and-wait.
5. `theme`: High complexity; reimplement for Linux UI toolkit, do not line-by-line port.
6. `dpi`: Medium complexity; mostly tied to Windows per-monitor DPI APIs.

## Windowing

Representative Win32 APIs:

- `wWinMain`
- `RegisterClass`
- `CreateWindowEx`
- `DefWindowProc`
- `SendMessage`
- `PostMessage`
- `SetWindowLongPtr` / `GetWindowLongPtr`
- `SetWindowPos`

Representative files:

- `PowerEditor/src/winmain.cpp`
- `PowerEditor/src/Notepad_plus_Window.cpp`
- `PowerEditor/src/NppBigSwitch.cpp`
- `PowerEditor/src/Notepad_plus.cpp`

Linux replacement direction:

- Introduce command dispatcher abstraction independent of Win32 messages.
- Replace HWND-centric state transfer with typed application events/commands.
- Let Qt event loop own lifecycle; bridge into app command layer.

Migration action:

- `Replace` (architecture-level), with temporary `Wrap` shims during transition.

## Filesystem

Representative Win32 APIs:

- `CreateFileW`
- `WriteFile`
- `MoveFileEx`
- `DeleteFile`
- `GetFileAttributesEx`
- `FindFirstFile` / `FindNextFile`
- `CreateDirectory`

Representative files:

- `PowerEditor/src/MISC/Common/FileInterface.cpp`
- `PowerEditor/src/ScintillaComponent/Buffer.cpp`
- `PowerEditor/src/NppIO.cpp`
- `PowerEditor/src/Parameters.cpp`
- `PowerEditor/src/MISC/Common/Common.cpp`

Linux replacement direction:

- `std::filesystem` for path/existence/rename/directory traversal.
- POSIX file descriptors or C++ stream abstraction for read/write paths where needed.
- Normalize all internal paths to UTF-8.
- Implement atomic-save behavior explicitly (`write temp` + `fsync` + `rename`).

Migration action:

- `Wrap` first, then `Replace` internals incrementally.

## Clipboard

Representative Win32 APIs:

- `OpenClipboard`
- `CloseClipboard`
- `GetClipboardData`
- `SetClipboardData`
- `EmptyClipboard`
- `CF_UNICODETEXT`

Representative files:

- `PowerEditor/src/NppCommands.cpp`
- `PowerEditor/src/Notepad_plus.cpp`
- `PowerEditor/src/MISC/Common/Common.cpp`
- `PowerEditor/src/ScintillaComponent/ScintillaEditView.cpp`

Linux replacement direction:

- Central `IClipboardService` abstraction.
- Qt backend via `QClipboard` for text and selection-mode behavior.
- Keep column/block selection metadata in internal format where needed.

Migration action:

- `Wrap` and migrate call sites to service API.

## Process and shell integration

Representative Win32 APIs:

- `ShellExecute` / `ShellExecuteExW`
- `WaitForSingleObject`

Representative files:

- `PowerEditor/src/NppCommands.cpp`
- `PowerEditor/src/MISC/Process/Processus.cpp`
- `PowerEditor/src/MISC/Common/Common.cpp`
- `PowerEditor/src/NppNotification.cpp`
- `PowerEditor/src/WinControls/StaticDialog/RunDlg/RunDlg.cpp`

Linux replacement direction:

- Split process concerns:
  - open URL/path with desktop handler (`xdg-open` or Qt desktop services)
  - spawn process with args/env/cwd
  - optional synchronous wait mode
- Use Qt process APIs (`QProcess`) in UI layer and/or POSIX process wrapper in platform layer.

Migration action:

- `Replace` with explicit process service contracts.

## Theme and visual styling

Representative Win32 APIs:

- `SetWindowTheme`
- `DwmSetWindowAttribute`
- `AllowDarkModeForWindow`
- `AllowDarkModeForApp`
- `SetPreferredAppMode`
- `uxtheme.dll`-linked behavior

Representative files:

- `PowerEditor/src/NppDarkMode.cpp`
- `PowerEditor/src/NppDarkMode.h`
- `PowerEditor/src/DarkMode/DarkMode.cpp`

Linux replacement direction:

- Rebuild theme behavior using Qt palette/style model.
- Keep semantic theme tokens (background/foreground/accent) rather than Win32 API parity.
- Do not preserve undocumented Windows-specific dark-mode hooks.

Migration action:

- `Replace` (no direct portability path expected).

## DPI and scaling

Representative Win32 APIs/messages:

- `GetDpiForWindow`
- `WM_DPICHANGED`
- `WM_DPICHANGED_AFTERPARENT`

Representative files:

- `PowerEditor/src/dpiManagerV2.cpp`
- `PowerEditor/src/NppDarkMode.cpp`

Linux replacement direction:

- Use Qt high-DPI support and per-screen scale factors.
- Convert DPI-specific calculations into toolkit-agnostic size/scaling helpers.

Migration action:

- `Replace` with toolkit-backed scaling layer.

## Suggested interface split (first pass)

Create these interfaces in `platform/include`:

- `IFileSystemService`
- `IClipboardService`
- `IProcessService`
- `IPathService` (XDG locations)
- `IThemeService` (semantic tokens and state)
- `IScalingService`

## Prioritized migration order

1. Windowing command flow abstraction.
2. Filesystem and path services.
3. Process service split.
4. Clipboard service.
5. DPI service adaptation.
6. Theme restyling.
