#!/usr/bin/env bash
set -euo pipefail

# Enforce that the Linux shipping/build source path stays free of Win32 APIs.

WIN32_PATTERN='(#include[[:space:]]*<[Ww]indows\.h>|(^|[^A-Za-z0-9_])(HWND|HMENU|HINSTANCE|LPARAM|WPARAM|LRESULT|HRESULT|DWORD|LPWSTR|HANDLE)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(SendMessage|PostMessage|CreateWindow(Ex)?|RegisterClass(Ex)?|DefWindowProc|ShellExecute(Ex|A|W)?|OpenClipboard|GetClipboardData|SetClipboardData|CloseClipboard)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(CreateFile(W|A)?|ReadFile|WriteFile|CloseHandle|DeleteFile(W|A)?|MoveFile(Ex)?(W|A)?|GetFileAttributes(W|A)?|FindFirstFile(W|A)?|FindNextFile(W|A)?|GetModuleFileName(W|A)?|OutputDebugString(W|A)?|MessageBox(W|A)?)([^A-Za-z0-9_]|$)|(^|[^A-Za-z0-9_])(Reg(Open|Create|Set|Get|Delete)Key(Ex)?W?|CoInitializeEx|CoCreateInstance)([^A-Za-z0-9_]|$))'

TARGET_DIRS=(
  "platform/include"
  "platform/linux"
  "ui/qt"
  "scintilla/src"
  "scintilla/qt"
  "lexilla/lexlib"
  "lexilla/lexers"
  "lexilla/src"
  "tests"
)

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "required command not found: ${cmd}" >&2
    exit 1
  fi
}

require_cmd rg

for dir in "${TARGET_DIRS[@]}"; do
  if [[ ! -d "${dir}" ]]; then
    echo "expected directory not found: ${dir}" >&2
    exit 1
  fi
done

set +e
matches="$(
  rg -n -H -e "${WIN32_PATTERN}" "${TARGET_DIRS[@]}" \
    --glob '*.c' \
    --glob '*.cc' \
    --glob '*.cpp' \
    --glob '*.cxx' \
    --glob '*.h' \
    --glob '*.hh' \
    --glob '*.hpp' \
    --glob '!tests/**/catch.hpp'
)"
rc=$?
set -e

if [[ "${rc}" -eq 0 ]]; then
  echo "failed: found Win32 usage in Linux shipping/build source path"
  echo
  echo "${matches}"
  exit 1
fi

if [[ "${rc}" -ne 1 ]]; then
  echo "failed: unable to run Win32 cleanliness scan (rg exit code ${rc})" >&2
  exit 1
fi

echo "pass: Linux shipping/build source path is free of direct Win32 usage"
