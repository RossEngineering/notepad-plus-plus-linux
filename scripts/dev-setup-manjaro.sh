#!/usr/bin/env bash
set -euo pipefail

if ! command -v pacman >/dev/null 2>&1; then
  echo "This setup script is intended for Manjaro/Arch (pacman not found)." >&2
  exit 1
fi

packages=(
  base-devel
  cmake
  ninja
  gcc
  pkgconf
  qt6-base
  qt6-tools
)

echo "Installing development dependencies..."
sudo pacman -S --needed "${packages[@]}"

echo "Configuring debug preset..."
cmake --preset debug

echo "Building debug preset..."
cmake --build --preset debug -- -j"$(nproc)"

echo "Running tests..."
ctest --preset debug --output-on-failure

cat <<'EOF'

Manjaro setup complete.
Run the app with:
  ./build/bin/notepad-plus-plus-linux

EOF
