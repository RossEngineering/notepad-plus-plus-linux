#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Validate desktop integration assets for a staged or installed tree.

Usage:
  scripts/linux/validate-desktop-integration.sh [--root <prefix>] [--strict]

Defaults:
  --root /usr

Examples:
  scripts/linux/validate-desktop-integration.sh --root /tmp/stage/usr
  scripts/linux/validate-desktop-integration.sh --root "$HOME/.local" --strict
EOF
}

root="/usr"
strict=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --root)
      root="$2"
      shift 2
      ;;
    --strict)
      strict=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

desktop_file="${root}/share/applications/notepad-plus-plus-linux.desktop"
mime_file="${root}/share/mime/packages/notepad-plus-plus-linux.xml"
icon_file="${root}/share/icons/hicolor/scalable/apps/notepad-plus-plus-linux.svg"
binary_file="${root}/bin/notepad-plus-plus-linux"

require_file() {
  local path="$1"
  if [[ ! -f "${path}" ]]; then
    echo "missing required file: ${path}" >&2
    exit 1
  fi
}

require_contains() {
  local path="$1"
  local pattern="$2"
  if ! grep -Fq "${pattern}" "${path}"; then
    echo "missing required pattern in ${path}: ${pattern}" >&2
    exit 1
  fi
}

require_file "${desktop_file}"
require_file "${mime_file}"
require_file "${icon_file}"
require_file "${binary_file}"

require_contains "${desktop_file}" "Type=Application"
require_contains "${desktop_file}" "Name=Notepad++ Linux"
if ! grep -Fq "Exec=notepad-plus-plus-linux %F" "${desktop_file}" && \
   ! grep -Fq "Exec=${root}/bin/notepad-plus-plus-linux %F" "${desktop_file}"; then
  echo "missing supported Exec entry in ${desktop_file}" >&2
  exit 1
fi
if ! grep -Fq "TryExec=notepad-plus-plus-linux" "${desktop_file}" && \
   ! grep -Fq "TryExec=${root}/bin/notepad-plus-plus-linux" "${desktop_file}"; then
  echo "missing supported TryExec entry in ${desktop_file}" >&2
  exit 1
fi
require_contains "${desktop_file}" "Icon=notepad-plus-plus-linux"
require_contains "${desktop_file}" "StartupWMClass=notepad-plus-plus-linux"
require_contains "${desktop_file}" "MimeType="
require_contains "${desktop_file}" "text/plain"
require_contains "${desktop_file}" "application/x-notepad-plus-plus-linux-session"

require_contains "${mime_file}" "application/x-notepad-plus-plus-linux-session"
require_contains "${mime_file}" "*.nppsession"

find_command_path() {
  local cmd="$1"
  local resolved=""
  if resolved="$(command -v "${cmd}" 2>/dev/null)"; then
    echo "${resolved}"
    return 0
  fi
  local sbin_candidate
  for sbin_candidate in /usr/sbin /usr/local/sbin /sbin; do
    if [[ -x "${sbin_candidate}/${cmd}" ]]; then
      echo "${sbin_candidate}/${cmd}"
      return 0
    fi
  done
  return 1
}

if command -v desktop-file-validate >/dev/null 2>&1; then
  desktop-file-validate "${desktop_file}"
elif [[ "${strict}" -eq 1 ]]; then
  echo "desktop-file-validate not found (strict mode)" >&2
  exit 1
fi

if command -v xmllint >/dev/null 2>&1; then
  xmllint --noout "${mime_file}"
elif [[ "${strict}" -eq 1 ]]; then
  echo "xmllint not found (strict mode)" >&2
  exit 1
fi

if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q "${root}/share/applications"
elif [[ "${strict}" -eq 1 ]]; then
  echo "update-desktop-database not found (strict mode)" >&2
  exit 1
fi

if command -v update-mime-database >/dev/null 2>&1; then
  update-mime-database "${root}/share/mime"
elif [[ "${strict}" -eq 1 ]]; then
  echo "update-mime-database not found (strict mode)" >&2
  exit 1
fi

icon_cache_cmd=""
if icon_cache_cmd="$(find_command_path gtk-update-icon-cache)"; then
  "${icon_cache_cmd}" -qtf "${root}/share/icons/hicolor" || true
elif icon_cache_cmd="$(find_command_path gtk4-update-icon-cache)"; then
  "${icon_cache_cmd}" -qtf "${root}/share/icons/hicolor" || true
elif [[ "${strict}" -eq 1 ]]; then
  echo "gtk-update-icon-cache/gtk4-update-icon-cache not found (strict mode)" >&2
  exit 1
fi

echo "desktop integration validation passed for root=${root}"
