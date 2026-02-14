#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Remove a user-local Notepad++ Linux install and refresh desktop/mime caches.

Usage:
  scripts/linux/uninstall-local.sh [--prefix <dir>]

Defaults:
  --prefix ~/.local
EOF
}

prefix="${HOME}/.local"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --prefix)
      prefix="$2"
      shift 2
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

echo "Removing from ${prefix}"

rm -f "${prefix}/bin/notepad-plus-plus-linux"
rm -f "${prefix}/share/applications/notepad-plus-plus-linux.desktop"
rm -f "${prefix}/share/icons/hicolor/scalable/apps/notepad-plus-plus-linux.svg"
rm -f "${prefix}/share/mime/packages/notepad-plus-plus-linux.xml"
rm -rf "${prefix}/share/notepad-plus-plus-linux"

cleanup_empty_dir() {
  local dir="$1"
  if [[ -d "${dir}" ]]; then
    rmdir --ignore-fail-on-non-empty "${dir}" 2>/dev/null || true
  fi
}

cleanup_empty_dir "${prefix}/share/icons/hicolor/scalable/apps"
cleanup_empty_dir "${prefix}/share/icons/hicolor/scalable"
cleanup_empty_dir "${prefix}/share/icons/hicolor"
cleanup_empty_dir "${prefix}/share/icons"
cleanup_empty_dir "${prefix}/share/mime/packages"
cleanup_empty_dir "${prefix}/share/mime"
cleanup_empty_dir "${prefix}/share/applications"
cleanup_empty_dir "${prefix}/share"
cleanup_empty_dir "${prefix}/bin"

update_if_present() {
  local cmd="$1"
  shift
  if command -v "${cmd}" >/dev/null 2>&1; then
    "${cmd}" "$@" || true
  fi
}

if [[ -d "${prefix}/share/applications" ]]; then
  update_if_present update-desktop-database -q "${prefix}/share/applications"
fi
if [[ -d "${prefix}/share/mime" ]]; then
  update_if_present update-mime-database "${prefix}/share/mime"
fi
if [[ -d "${prefix}/share/icons/hicolor" ]]; then
  update_if_present gtk-update-icon-cache -qtf "${prefix}/share/icons/hicolor"
fi

echo "Uninstall complete."
