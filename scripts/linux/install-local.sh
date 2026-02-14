#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Install Notepad++ Linux into a user-local prefix and register desktop integration.

Usage:
  scripts/linux/install-local.sh [--from-stage <dir>] [--from-tar <file>] [--prefix <dir>] [--set-default]

Defaults:
  --from-stage out/release/stage
  --prefix ~/.local

Options:
  --from-stage <dir>   Use an existing install stage directory containing usr/bin, usr/share, ...
  --from-tar <file>    Extract a release tar.xz and install from its root.
  --prefix <dir>       Install prefix (recommended: ~/.local for user installs).
  --set-default        Register notepad-plus-plus-linux.desktop as default for common text mime types.
  -h, --help           Show this help.
EOF
}

stage_dir="out/release/stage"
tar_file=""
prefix="${HOME}/.local"
set_default=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --from-stage)
      stage_dir="$2"
      shift 2
      ;;
    --from-tar)
      tar_file="$2"
      shift 2
      ;;
    --prefix)
      prefix="$2"
      shift 2
      ;;
    --set-default)
      set_default=1
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

if [[ -n "${tar_file}" && "${stage_dir}" != "out/release/stage" ]]; then
  echo "use only one source: --from-stage or --from-tar" >&2
  exit 2
fi

tmp_dir=""
cleanup() {
  if [[ -n "${tmp_dir}" && -d "${tmp_dir}" ]]; then
    rm -rf "${tmp_dir}"
  fi
}
trap cleanup EXIT

if [[ -n "${tar_file}" ]]; then
  if [[ ! -f "${tar_file}" ]]; then
    echo "release tarball not found: ${tar_file}" >&2
    exit 1
  fi
  tmp_dir="$(mktemp -d)"
  tar -xJf "${tar_file}" -C "${tmp_dir}"
  stage_dir="${tmp_dir}"
fi

if [[ ! -d "${stage_dir}/usr/bin" || ! -d "${stage_dir}/usr/share" ]]; then
  echo "invalid install source: expected ${stage_dir}/usr/bin and ${stage_dir}/usr/share" >&2
  exit 1
fi

echo "Installing into ${prefix}"
mkdir -p "${prefix}/bin" "${prefix}/share"
cp -a "${stage_dir}/usr/bin/." "${prefix}/bin/"
cp -a "${stage_dir}/usr/share/." "${prefix}/share/"

desktop_entry="${prefix}/share/applications/notepad-plus-plus-linux.desktop"
binary_target="${prefix}/bin/notepad-plus-plus-linux"
if [[ -f "${desktop_entry}" ]]; then
  escaped_binary_target="${binary_target//\//\\/}"
  sed -i "s|^Exec=.*$|Exec=${escaped_binary_target} %F|" "${desktop_entry}"
  sed -i "s|^TryExec=.*$|TryExec=${escaped_binary_target}|" "${desktop_entry}"
fi

update_if_present() {
  local cmd="$1"
  shift
  if command -v "${cmd}" >/dev/null 2>&1; then
    "${cmd}" "$@" || true
  fi
}

update_if_present update-desktop-database -q "${prefix}/share/applications"
update_if_present update-mime-database "${prefix}/share/mime"
update_if_present gtk-update-icon-cache -qtf "${prefix}/share/icons/hicolor"

if [[ "${set_default}" -eq 1 ]]; then
  if command -v xdg-mime >/dev/null 2>&1; then
    for mime in \
      text/plain \
      text/markdown \
      application/json \
      application/xml \
      text/x-shellscript \
      text/x-python \
      text/x-csrc \
      text/x-c++src; do
      DE=generic XDG_CURRENT_DESKTOP= \
        xdg-mime default notepad-plus-plus-linux.desktop "${mime}" || true
    done
  else
    echo "xdg-mime not found; skipped default handler registration"
  fi
fi

echo
echo "Install complete."
echo "Binary: ${prefix}/bin/notepad-plus-plus-linux"
echo "Desktop entry: ${prefix}/share/applications/notepad-plus-plus-linux.desktop"
echo "If needed, add ${prefix}/bin to PATH."
