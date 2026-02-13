#!/usr/bin/env bash
set -euo pipefail

BASE_REF="${1:-baseline-2026-02-13}"

if ! git rev-parse --verify "$BASE_REF" >/dev/null 2>&1; then
    echo "error: base ref '$BASE_REF' not found"
    exit 2
fi

WIN32_PATTERN='(#include[[:space:]]*<[Ww]indows\.h>|(^|[^A-Za-z0-9_])(HWND|HMENU|HINSTANCE|LPARAM|WPARAM|LRESULT|HRESULT|DWORD|LPWSTR)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(SendMessage|PostMessage|CreateWindow(Ex)?|RegisterClass(Ex)?|DefWindowProc|ShellExecute(Ex|A|W)?|OpenClipboard|GetClipboardData|SetClipboardData|CloseClipboard)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(CreateFile(W|A)?|ReadFile|WriteFile|CloseHandle|DeleteFile(W|A)?|MoveFile(Ex)?(W|A)?|GetFileAttributes(W|A)?|FindFirstFile(W|A)?|FindNextFile(W|A)?|GetModuleFileName(W|A)?|OutputDebugString(W|A)?|MessageBox(W|A)?)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(Reg(Open|Create|Set|Get|Delete)Key(Ex)?W?|CoInitializeEx|CoCreateInstance)([^A-Za-z0-9_]|$))'

TARGET_GLOBS=(
    "*.c"
    "*.cc"
    "*.cpp"
    "*.cxx"
    "*.h"
    "*.hh"
    "*.hpp"
    "*.rc"
)

violations=0

while IFS= read -r file; do
    [[ -z "$file" ]] && continue

    # Platform layer is the only allowed location for new Win32-specific additions.
    if [[ "$file" == platform/win32/* ]]; then
        continue
    fi

    added_lines="$(
        git diff --no-color --unified=0 "$BASE_REF"...HEAD -- "$file" \
        | sed -n '/^\+[^+]/p' \
        | sed 's/^+//'
    )"

    [[ -z "$added_lines" ]] && continue

    if matches=$(printf '%s\n' "$added_lines" | grep -En "$WIN32_PATTERN"); then
        echo "win32-boundary violation in $file"
        echo "$matches"
        echo
        violations=1
    fi
done < <(git diff --name-only "$BASE_REF"...HEAD -- "${TARGET_GLOBS[@]}")

if [[ "$violations" -ne 0 ]]; then
    echo "failed: found newly added Win32 usage outside platform/win32"
    exit 1
fi

echo "pass: no new Win32 usage outside platform/win32"
