# Coding Rules for Linux Migration

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13

## Rule 1: No new direct Win32 usage outside platform layer

Do not introduce new direct Win32 APIs, Win32 message calls, or Win32-specific types outside `platform/win32`.

Examples of disallowed additions outside `platform/win32`:

- `#include <windows.h>`
- `HWND`, `HMENU`, `HINSTANCE`, `LPARAM`, `WPARAM`, `LRESULT`, `HRESULT`, `DWORD`
- `SendMessage`, `PostMessage`, `CreateWindowEx`, `RegisterClass`, `DefWindowProc`
- `ShellExecute`, `OpenClipboard`, `GetClipboardData`, `SetClipboardData`
- `CreateFileW`, `ReadFile`, `WriteFile`, `CloseHandle`, `DeleteFileW`, `MoveFileExW`, `GetFileAttributesW`
- `GetModuleFileNameW`, `OutputDebugStringW`, `MessageBoxW`, `RegOpenKeyExW`, `CoInitializeEx`

## Required approach

1. Add or use an interface in `platform/include`.
2. Implement Linux behavior under `platform/linux`.
3. (Optional) Keep Win32-specific implementation under `platform/win32` when dual-platform support is needed.
4. Consume only interface types from `core` and `ui` code.

## Scope note

Legacy Win32 code still exists in `PowerEditor/` during migration. This rule is forward-looking: no new Win32 coupling should be added outside the designated platform layer.

## Automated enforcement

- Script: `scripts/check_win32_boundaries.sh`
- CI workflow: `.github/workflows/win32-boundary.yml`
