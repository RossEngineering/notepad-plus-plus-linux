#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Validate default file-handler associations for a desktop entry.

Usage:
  scripts/linux/validate-file-handler-defaults.sh --desktop-id <id> [mime...]

Examples:
  scripts/linux/validate-file-handler-defaults.sh \
    --desktop-id notepad-plus-plus-linux.desktop \
    text/plain text/markdown application/json
EOF
}

desktop_id=""
declare -a mime_types=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --desktop-id)
      desktop_id="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      mime_types+=("$1")
      shift
      ;;
  esac
done

if [[ -z "${desktop_id}" ]]; then
  echo "missing required argument: --desktop-id" >&2
  usage >&2
  exit 2
fi

if [[ ${#mime_types[@]} -eq 0 ]]; then
  mime_types=(
    text/plain
    text/markdown
    application/json
    application/xml
    text/x-shellscript
    text/x-python
    text/x-csrc
    text/x-c++src
  )
fi

if ! command -v xdg-mime >/dev/null 2>&1; then
  echo "xdg-mime not found" >&2
  exit 1
fi

for mime in "${mime_types[@]}"; do
  resolved="$(
    DE=generic XDG_CURRENT_DESKTOP= \
      xdg-mime query default "${mime}" 2>/dev/null || true
  )"
  if [[ "${resolved}" != "${desktop_id}" ]]; then
    echo "default handler mismatch for ${mime}: expected ${desktop_id}, got ${resolved:-<none>}" >&2
    exit 1
  fi
done

echo "file-handler default validation passed for ${desktop_id}"
