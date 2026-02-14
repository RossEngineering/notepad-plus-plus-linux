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

desktop_id="notepad-plus-plus-linux.desktop"

remove_mime_defaults() {
  local mimeapps_file="${XDG_CONFIG_HOME:-$HOME/.config}/mimeapps.list"
  if [[ ! -f "${mimeapps_file}" ]]; then
    return 0
  fi

  local mime
  for mime in \
    text/plain \
    text/markdown \
    application/json \
    application/xml \
    text/x-shellscript \
    text/x-python \
    text/x-csrc \
    text/x-c++src; do
    local tmp_file
    tmp_file="$(mktemp)"
    awk -v key="${mime}=" -v desktop="${desktop_id}" '
      index($0, key) == 1 && index($0, desktop) > 0 { next }
      { print }
    ' "${mimeapps_file}" > "${tmp_file}"
    mv "${tmp_file}" "${mimeapps_file}"
  done
}

rm -f "${prefix}/bin/notepad-plus-plus-linux"
rm -f "${prefix}/share/applications/notepad-plus-plus-linux.desktop"
rm -f "${prefix}/share/icons/hicolor/scalable/apps/notepad-plus-plus-linux.svg"
rm -f "${prefix}/share/mime/packages/notepad-plus-plus-linux.xml"
rm -rf "${prefix}/share/notepad-plus-plus-linux"

remove_mime_defaults

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
  local resolved=""
  if resolved="$(command -v "${cmd}" 2>/dev/null)"; then
    "${resolved}" "$@" || true
    return 0
  fi
  local sbin_candidate
  for sbin_candidate in /usr/sbin /usr/local/sbin /sbin; do
    if [[ -x "${sbin_candidate}/${cmd}" ]]; then
      "${sbin_candidate}/${cmd}" "$@" || true
      return 0
    fi
  done
  return 1
}

update_icon_cache_if_present() {
  local icon_root="$1"
  if ! update_if_present gtk-update-icon-cache -qtf "${icon_root}"; then
    update_if_present gtk4-update-icon-cache -qtf "${icon_root}"
  fi
}

if [[ -d "${prefix}/share/applications" ]]; then
  update_if_present update-desktop-database -q "${prefix}/share/applications"
fi
if [[ -d "${prefix}/share/mime/packages" ]]; then
  update_if_present update-mime-database "${prefix}/share/mime"
fi
if [[ -d "${prefix}/share/icons/hicolor" ]]; then
  update_icon_cache_if_present "${prefix}/share/icons/hicolor"
fi

echo "Uninstall complete."
