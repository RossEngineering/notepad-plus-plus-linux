#!/usr/bin/env bash
set -euo pipefail

if ! command -v docker >/dev/null 2>&1; then
  echo "docker not found" >&2
  exit 1
fi

docker run --rm \
  -v "$(pwd):/work" \
  -w /work \
  ubuntu:24.04 \
  bash -lc '
    set -euo pipefail
    apt-get update
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
      cmake \
      ninja-build \
      g++ \
      git \
      pkg-config \
      qt6-base-dev \
      qt6-5compat-dev \
      qt6-wayland \
      weston \
      xdg-utils

    cmake -S . -B /tmp/npp-wayland-build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build /tmp/npp-wayland-build --target npp_linux_shell -- -j2

    export XDG_RUNTIME_DIR=/tmp/npp-wayland-runtime
    mkdir -p "${XDG_RUNTIME_DIR}"
    chmod 700 "${XDG_RUNTIME_DIR}"
    export WAYLAND_DISPLAY=wayland-1

    weston_log=/tmp/npp-wayland-weston.log
    weston --backend=headless-backend.so \
      --socket="${WAYLAND_DISPLAY}" \
      --idle-time=0 \
      --no-config \
      --log="${weston_log}" >/tmp/npp-wayland-weston.stdout 2>&1 &
    weston_pid=$!
    cleanup() {
      kill "${weston_pid}" >/dev/null 2>&1 || true
      wait "${weston_pid}" >/dev/null 2>&1 || true
    }
    trap cleanup EXIT

    ready=0
    for _ in $(seq 1 50); do
      if [[ -S "${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY}" ]]; then
        ready=1
        break
      fi
      sleep 0.1
    done

    if [[ "${ready}" -ne 1 ]]; then
      echo "wayland compositor socket did not appear: ${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY}" >&2
      cat /tmp/npp-wayland-weston.stdout >&2 || true
      cat "${weston_log}" >&2 || true
      exit 1
    fi

    app_log=/tmp/npp-wayland-app.log
    set +e
    timeout 12s env \
      QT_QPA_PLATFORM=wayland \
      WAYLAND_DISPLAY="${WAYLAND_DISPLAY}" \
      /tmp/npp-wayland-build/bin/notepad-plus-plus-linux --new-file >"${app_log}" 2>&1
    app_rc=$?
    set -e

    if [[ "${app_rc}" -ne 124 ]]; then
      echo "notepad-plus-plus-linux did not remain healthy under Wayland smoke timeout (exit=${app_rc})" >&2
      cat "${app_log}" >&2 || true
      cat "${weston_log}" >&2 || true
      exit 1
    fi

    if grep -Eqi \
      "no Qt platform plugin could be initialized|Could not find the Qt platform plugin|qt.qpa.wayland|segmentation fault|core dumped" \
      "${app_log}"; then
      echo "detected Wayland startup/runtime errors in app log" >&2
      cat "${app_log}" >&2 || true
      exit 1
    fi
  '

echo "Wayland QA smoke pass complete"
