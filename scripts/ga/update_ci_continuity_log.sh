#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOG_FILE="${ROOT_DIR}/docs/ga-ci-continuity-log-2026-02.md"

WORKFLOW_CMAKE="Linux CMake Build"
WORKFLOW_DESKTOP="Linux Desktop Integration"
WORKFLOW_WIN32="Win32 Boundary Guard"

usage() {
  cat <<'EOF'
Append one GA CI continuity row to docs/ga-ci-continuity-log-2026-02.md.

Usage:
  scripts/ga/update_ci_continuity_log.sh [--commit <sha>] [--date YYYY-MM-DD] [--dry-run]

Defaults:
  --commit  HEAD
  --date    inferred from workflow run timestamp (UTC)

Behavior:
  1. Fetches the latest run for each required workflow at the target commit.
  2. Computes day result using existing continuity rules:
     - Counted (n/7) for first green commit of a UTC day.
     - Pass (same day, continuity unchanged) for additional same-day green commits.
     - Reset (0/7) when any required lane fails.
  3. Appends a markdown table row directly after the latest continuity row.
EOF
}

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "required command not found: ${cmd}" >&2
    exit 1
  fi
}

commit_ref="HEAD"
date_override=""
dry_run=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --commit)
      commit_ref="$2"
      shift 2
      ;;
    --date)
      date_override="$2"
      shift 2
      ;;
    --dry-run)
      dry_run=1
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

require_cmd git
require_cmd gh
require_cmd awk

if [[ ! -f "${LOG_FILE}" ]]; then
  echo "continuity log not found: ${LOG_FILE}" >&2
  exit 1
fi

if ! commit_full="$(git -C "${ROOT_DIR}" rev-parse --verify "${commit_ref}^{commit}" 2>/dev/null)"; then
  echo "invalid commit reference: ${commit_ref}" >&2
  exit 2
fi
commit_short="$(git -C "${ROOT_DIR}" rev-parse --short=9 "${commit_full}")"

if grep -Fq "| \`${commit_short}\` |" "${LOG_FILE}"; then
  echo "continuity row already exists for commit ${commit_short}; nothing to do"
  exit 0
fi

fetch_run() {
  local workflow="$1"
  local raw
  if ! raw="$(
    gh run list \
      --workflow "${workflow}" \
      --commit "${commit_full}" \
      --limit 1 \
      --json databaseId,conclusion,headSha,createdAt \
      --jq 'if length == 0 then "" else "\(.[0].databaseId)|\(.[0].conclusion)|\(.[0].headSha)|\(.[0].createdAt)" end'
  )"; then
    echo "failed to query workflow '${workflow}' for commit ${commit_short}" >&2
    return 1
  fi

  if [[ -z "${raw}" ]]; then
    echo "no run found for workflow '${workflow}' at commit ${commit_short}" >&2
    return 1
  fi

  local run_id run_conclusion run_sha run_created_at
  IFS='|' read -r run_id run_conclusion run_sha run_created_at <<< "${raw}"

  if [[ -z "${run_conclusion}" ]]; then
    echo "workflow '${workflow}' is still in progress for commit ${commit_short}" >&2
    return 1
  fi

  local run_status="Fail"
  if [[ "${run_conclusion}" == "success" ]]; then
    run_status="Pass"
  fi

  printf '%s|%s|%s|%s\n' "${run_id}" "${run_status}" "${run_sha}" "${run_created_at}"
}

if ! cmake_run="$(fetch_run "${WORKFLOW_CMAKE}")"; then
  exit 1
fi
if ! desktop_run="$(fetch_run "${WORKFLOW_DESKTOP}")"; then
  exit 1
fi
if ! win32_run="$(fetch_run "${WORKFLOW_WIN32}")"; then
  exit 1
fi

IFS='|' read -r cmake_id cmake_status cmake_sha cmake_created_at <<< "${cmake_run}"
IFS='|' read -r desktop_id desktop_status desktop_sha _desktop_created_at <<< "${desktop_run}"
IFS='|' read -r win32_id win32_status win32_sha _win32_created_at <<< "${win32_run}"

if [[ "${cmake_sha}" != "${desktop_sha}" || "${cmake_sha}" != "${win32_sha}" ]]; then
  echo "workflow run SHA mismatch for commit ${commit_short}" >&2
  echo "  ${WORKFLOW_CMAKE}: ${cmake_sha}" >&2
  echo "  ${WORKFLOW_DESKTOP}: ${desktop_sha}" >&2
  echo "  ${WORKFLOW_WIN32}: ${win32_sha}" >&2
  exit 1
fi

if [[ -n "${date_override}" ]]; then
  if [[ ! "${date_override}" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}$ ]]; then
    echo "invalid --date value: ${date_override} (expected YYYY-MM-DD)" >&2
    exit 2
  fi
  date_utc="${date_override}"
else
  date_utc="${cmake_created_at%%T*}"
fi

last_date="$(
  awk -F'|' '
    /^\| [0-9]{4}-[0-9]{2}-[0-9]{2} / {
      date=$2
      gsub(/^[ \t]+|[ \t]+$/, "", date)
      last_date=date
    }
    END { print last_date }
  ' "${LOG_FILE}"
)"

previous_streak="$(
  awk -F'|' '
    BEGIN { streak=0 }
    /^\| [0-9]{4}-[0-9]{2}-[0-9]{2} / {
      result=$7
      gsub(/^[ \t]+|[ \t]+$/, "", result)
      if (result ~ /^Counted \([0-9]+\/7\)$/) {
        sub(/^Counted \(/, "", result)
        sub(/\/7\)$/, "", result)
        streak=result + 0
      } else if (result == "Reset (0/7)") {
        streak=0
      }
    }
    END { print streak }
  ' "${LOG_FILE}"
)"

all_pass=0
if [[ "${cmake_status}" == "Pass" && "${desktop_status}" == "Pass" && "${win32_status}" == "Pass" ]]; then
  all_pass=1
fi

if (( all_pass == 1 )); then
  if [[ -n "${last_date}" && "${last_date}" == "${date_utc}" ]]; then
    day_result="Pass (same day, continuity unchanged)"
    streak_after="${previous_streak}"
  else
    streak_after=$((previous_streak + 1))
    if (( streak_after > 7 )); then
      streak_after=7
    fi
    day_result="Counted (${streak_after}/7)"
  fi
else
  day_result="Reset (0/7)"
  streak_after=0
fi

new_row="| ${date_utc} | \`${commit_short}\` | ${cmake_status} | ${desktop_status} | ${win32_status} | ${day_result} | \`${cmake_id}\`, \`${desktop_id}\`, \`${win32_id}\` |"

if (( dry_run == 1 )); then
  echo "dry-run: would append row:"
  echo "${new_row}"
  exit 0
fi

tmp_file="$(mktemp)"
trap 'rm -f "${tmp_file}"' EXIT

last_row_number="$(
  grep -nE '^\| [0-9]{4}-[0-9]{2}-[0-9]{2} ' "${LOG_FILE}" \
    | tail -1 \
    | cut -d: -f1
)"

if [[ -z "${last_row_number}" ]]; then
  echo "no continuity data rows found in ${LOG_FILE}" >&2
  exit 1
fi

awk -v row="${new_row}" -v row_number="${last_row_number}" '
  { print }
  NR == row_number { print row }
' "${LOG_FILE}" > "${tmp_file}"

mv "${tmp_file}" "${LOG_FILE}"

echo "updated ${LOG_FILE}"
echo "  row: ${new_row}"
echo "  streak after update: ${streak_after}/7"
