#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CREATE_SCRIPT="${ROOT_DIR}/scripts/release/create_linux_release_artifacts.sh"

usage() {
  cat <<'EOF'
Run release engineering dry-run checks for RC3 readiness.

Usage:
  scripts/release/run_release_dry_run.sh [--version <label>] [--output-dir <dir>] [--signing-mode <auto|require|skip>]

Defaults:
  --version rc3-dryrun-<short-sha>
  --output-dir out/release-dry-run
  --signing-mode auto

Environment:
  LINUX_RELEASE_GPG_KEY         Optional base64-encoded private key for signing.
  LINUX_RELEASE_GPG_PASSPHRASE  Optional passphrase for imported key.

Behavior:
  1. Validates rollback restore mechanics for output directory replacement.
  2. Generates release artifacts and verifies checksum integrity.
  3. Verifies reproducibility by rebuilding artifacts in a second output directory and comparing checksums.
  4. Validates signing path:
     - uses LINUX_RELEASE_GPG_KEY if present
     - otherwise generates a temporary dry-run key when signing-mode is auto
EOF
}

version=""
output_dir="${ROOT_DIR}/out/release-dry-run"
signing_mode="auto"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --version)
      version="$2"
      shift 2
      ;;
    --output-dir)
      output_dir="$2"
      shift 2
      ;;
    --signing-mode)
      signing_mode="$2"
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

if [[ "${signing_mode}" != "auto" && "${signing_mode}" != "require" && "${signing_mode}" != "skip" ]]; then
  echo "invalid --signing-mode: ${signing_mode}" >&2
  exit 2
fi

if [[ -z "${version}" ]]; then
  short_sha="$(git -C "${ROOT_DIR}" rev-parse --short HEAD)"
  version="rc3-dryrun-${short_sha}"
fi

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "required command not found: ${cmd}" >&2
    exit 1
  fi
}

require_cmd cmake
require_cmd ninja
require_cmd tar
require_cmd xz
require_cmd sha256sum

run_create_artifacts() {
  local run_version="$1"
  local run_output_dir="$2"
  local run_build_dir
  run_build_dir="$(mktemp -d)"
  NPP_RELEASE_BUILD_DIR="${run_build_dir}" "${CREATE_SCRIPT}" "${run_version}" "${run_output_dir}"
  rm -rf "${run_build_dir}"
}

prepare_output_dir() {
  local dir="$1"
  local backup=""
  if [[ -d "${dir}" ]]; then
    backup="${dir}.bak.$(date +%s)"
    mv "${dir}" "${backup}"
  fi
  mkdir -p "${dir}"
  echo "${backup}"
}

rollback_output_dir() {
  local dir="$1"
  local backup="$2"
  rm -rf "${dir}"
  if [[ -n "${backup}" && -d "${backup}" ]]; then
    mv "${backup}" "${dir}"
  fi
}

commit_output_dir() {
  local backup="$1"
  if [[ -n "${backup}" && -d "${backup}" ]]; then
    rm -rf "${backup}"
  fi
}

validate_rollback_probe() {
  local probe_dir="${ROOT_DIR}/out/release-rollback-probe"
  rm -rf "${probe_dir}" "${probe_dir}.bak."*
  mkdir -p "${probe_dir}"
  echo "previous-state" > "${probe_dir}/rollback-marker.txt"

  local probe_backup
  probe_backup="$(prepare_output_dir "${probe_dir}")"
  echo "partial-state" > "${probe_dir}/partial.txt"
  rollback_output_dir "${probe_dir}" "${probe_backup}"

  if [[ ! -f "${probe_dir}/rollback-marker.txt" ]]; then
    echo "rollback probe failed: marker file missing after restore" >&2
    return 1
  fi
  if [[ "$(cat "${probe_dir}/rollback-marker.txt")" != "previous-state" ]]; then
    echo "rollback probe failed: marker content mismatch" >&2
    return 1
  fi

  rm -rf "${probe_dir}"
  echo "rollback probe: pass"
}

validate_rollback_probe

source_date_epoch="$(git -C "${ROOT_DIR}" log -1 --pretty=%ct)"
export SOURCE_DATE_EPOCH="${source_date_epoch}"

output_backup=""
cleanup_on_error() {
  local rc=$?
  if (( rc != 0 )); then
    rollback_output_dir "${output_dir}" "${output_backup}"
  fi
  exit "${rc}"
}
trap cleanup_on_error EXIT

output_backup="$(prepare_output_dir "${output_dir}")"

echo "building primary dry-run artifacts..."
run_create_artifacts "${version}" "${output_dir}"

artifact_base="notepad-plus-plus-linux-${version}-x86_64"
tarball="${output_dir}/${artifact_base}.tar.xz"
checksum_file="${output_dir}/${artifact_base}.sha256"

if [[ ! -f "${tarball}" || ! -f "${checksum_file}" ]]; then
  echo "dry-run failed: expected artifact files were not created" >&2
  exit 1
fi

(
  cd "${output_dir}"
  sha256sum -c "${artifact_base}.sha256"
)
echo "checksum verification: pass"

repro_dir="$(mktemp -d)"
repro_output="${repro_dir}/out"

echo "building reproducibility comparison artifacts..."
run_create_artifacts "${version}" "${repro_output}"

primary_hash="$(cut -d ' ' -f 1 "${checksum_file}")"
repro_hash="$(cut -d ' ' -f 1 "${repro_output}/${artifact_base}.sha256")"
if [[ "${primary_hash}" != "${repro_hash}" ]]; then
  echo "reproducibility check failed: checksum mismatch" >&2
  echo "primary: ${primary_hash}" >&2
  echo "rebuild: ${repro_hash}" >&2
  exit 1
fi
echo "reproducibility check: pass"

run_signing=0
generated_ephemeral_key=0
if [[ "${signing_mode}" == "skip" ]]; then
  run_signing=0
elif [[ -n "${LINUX_RELEASE_GPG_KEY:-}" ]]; then
  run_signing=1
elif [[ "${signing_mode}" == "require" ]]; then
  echo "signing-mode=require but LINUX_RELEASE_GPG_KEY is empty" >&2
  exit 1
else
  run_signing=1
  generated_ephemeral_key=1
fi

if (( run_signing == 1 )); then
  require_cmd gpg
  gpg_home="$(mktemp -d)"
  export GNUPGHOME="${gpg_home}"
  chmod 700 "${GNUPGHOME}"

  if (( generated_ephemeral_key == 1 )); then
    gpg --batch --pinentry-mode loopback --passphrase '' \
      --quick-generate-key "NPP Dry Run <dryrun@example.invalid>" ed25519 sign 1d
    key_id="$(gpg --list-secret-keys --with-colons | awk -F: '/^sec:/ {print $5; exit}')"
    if [[ -z "${key_id}" ]]; then
      echo "failed to generate ephemeral signing key" >&2
      exit 1
    fi
    gpg_passphrase=""
  else
    echo "${LINUX_RELEASE_GPG_KEY}" | base64 -d | gpg --batch --import
    key_id="$(gpg --list-secret-keys --with-colons | awk -F: '/^sec:/ {print $5; exit}')"
    if [[ -z "${key_id}" ]]; then
      echo "failed to import signing key" >&2
      exit 1
    fi
    gpg_passphrase="${LINUX_RELEASE_GPG_PASSPHRASE:-}"
  fi

  shopt -s nullglob
  for to_sign in "${tarball}" "${checksum_file}"; do
    gpg --batch --yes --pinentry-mode loopback \
      --passphrase "${gpg_passphrase}" \
      --armor --detach-sign "${to_sign}"
  done

  for signed_artifact in "${tarball}" "${checksum_file}"; do
    gpg --verify "${signed_artifact}.asc" "${signed_artifact}"
  done
  echo "signing verification: pass"

  rm -rf "${GNUPGHOME}"
fi

rm -rf "${repro_dir}"
commit_output_dir "${output_backup}"
trap - EXIT

echo
echo "release dry-run passed"
echo "version=${version}"
echo "output_dir=${output_dir}"
echo "artifact=${tarball}"
echo "checksum=${checksum_file}"
if (( run_signing == 1 )); then
  if (( generated_ephemeral_key == 1 )); then
    echo "signing=pass (ephemeral dry-run key)"
  else
    echo "signing=pass (provided release key)"
  fi
else
  echo "signing=skipped"
fi
