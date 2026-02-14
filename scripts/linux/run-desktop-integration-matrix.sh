#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Run desktop + file-handler integration validation in a distro container.

Usage:
  scripts/linux/run-desktop-integration-matrix.sh <arch-derivative|ubuntu-lts|fedora-stable>
EOF
}

if [[ $# -ne 1 ]]; then
  usage >&2
  exit 2
fi

distro="$1"

if ! command -v docker >/dev/null 2>&1; then
  echo "docker not found" >&2
  exit 1
fi

case "${distro}" in
  arch-derivative)
    image="archlinux:latest"
    setup_cmd='pacman -Syu --noconfirm --needed base-devel cmake ninja git pkgconf qt6-base qt6-5compat desktop-file-utils shared-mime-info libxml2 xdg-utils'
    ;;
  ubuntu-lts)
    image="ubuntu:24.04"
    setup_cmd='apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y cmake ninja-build g++ git pkg-config qt6-base-dev qt6-5compat-dev desktop-file-utils shared-mime-info libxml2-utils xdg-utils'
    ;;
  fedora-stable)
    image="fedora:41"
    setup_cmd='dnf -y install cmake ninja-build gcc-c++ git pkgconf-pkg-config qt6-qtbase-devel qt6-qt5compat-devel desktop-file-utils shared-mime-info libxml2 xdg-utils && dnf clean all'
    ;;
  *)
    echo "unsupported distro key: ${distro}" >&2
    usage >&2
    exit 2
    ;;
esac

docker run --rm \
  -v "$(pwd):/work" \
  -w /work \
  "${image}" \
  bash -lc "
    set -euo pipefail
    ${setup_cmd}
    prefix=/tmp/notepad-plus-plus-linux-${distro}
    export XDG_CONFIG_HOME=/tmp/npp-xdg-config
    export XDG_DATA_HOME=\"\${prefix}/share\"
    mkdir -p \"\${XDG_CONFIG_HOME}\" \"\${XDG_DATA_HOME}\"
    ./scripts/release/create_linux_release_artifacts.sh ci-${distro}
    ./scripts/linux/validate-desktop-integration.sh --root /work/out/release/stage/usr --strict
    ./scripts/linux/install-local.sh --from-stage /work/out/release/stage --prefix \"\${prefix}\" --set-default
    ./scripts/linux/validate-desktop-integration.sh --root \"\${prefix}\" --strict
    ./scripts/linux/validate-file-handler-defaults.sh --desktop-id notepad-plus-plus-linux.desktop
    ./scripts/linux/uninstall-local.sh --prefix \"\${prefix}\"
  "

echo "desktop/file-handler integration matrix validation passed for ${distro}"
