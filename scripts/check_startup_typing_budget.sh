#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "usage: $0 <benchmark-executable> [benchmark-args...]" >&2
  exit 2
fi

benchmark_executable="$1"
shift || true

if [[ ! -x "${benchmark_executable}" ]]; then
  echo "benchmark executable not found or not executable: ${benchmark_executable}" >&2
  exit 2
fi

startup_mean_max_us="${NPP_STARTUP_MEAN_MAX_US:-200.0}"
startup_p95_max_us="${NPP_STARTUP_P95_MAX_US:-300.0}"
typing_mean_max_us="${NPP_TYPING_MEAN_MAX_US:-5.0}"
typing_p95_max_us="${NPP_TYPING_P95_MAX_US:-10.0}"

metrics="$("${benchmark_executable}" "$@")"
echo "${metrics}"

extract_metric() {
  local key="$1"
  echo "${metrics}" | awk -F= -v metric_key="${key}" '$1 == metric_key { print $2; found=1 } END { if (!found) exit 1 }'
}

startup_mean="$(extract_metric "startup_mean_us")"
startup_p95="$(extract_metric "startup_p95_us")"
typing_mean="$(extract_metric "typing_mean_us")"
typing_p95="$(extract_metric "typing_p95_us")"

assert_within_budget() {
  local label="$1"
  local observed="$2"
  local threshold="$3"
  if ! awk -v actual="${observed}" -v limit="${threshold}" 'BEGIN { exit !(actual <= limit) }'; then
    echo "budget check failed: ${label}=${observed} exceeds threshold ${threshold}" >&2
    exit 1
  fi
}

assert_within_budget "startup_mean_us" "${startup_mean}" "${startup_mean_max_us}"
assert_within_budget "startup_p95_us" "${startup_p95}" "${startup_p95_max_us}"
assert_within_budget "typing_mean_us" "${typing_mean}" "${typing_mean_max_us}"
assert_within_budget "typing_p95_us" "${typing_p95}" "${typing_p95_max_us}"

echo "performance budgets satisfied"
