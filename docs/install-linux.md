# Linux Install Guide

Last updated: 2026-02-13

This guide covers install/build paths for the currently validated distro matrix:

1. Arch Linux and derivatives (Manjaro baseline)
2. Ubuntu LTS (24.04+)
3. Fedora stable

## From source (common flow)

```bash
cmake --preset release
cmake --build --preset release -- -j"$(nproc)"
ctest --preset release --output-on-failure
sudo cmake --install build/release
```

Installed binary path (default):

- `/usr/local/bin/notepad-plus-plus-linux`

## Arch Linux and derivatives

### Build dependencies

```bash
sudo pacman -S --needed base-devel cmake ninja gcc pkgconf qt6-base qt6-tools
```

### Package install path (`PKGBUILD`)

```bash
makepkg -si
```

Primary packaging files:

- `packaging/arch/PKGBUILD`
- `packaging/arch/notepad-plus-plus-linux.install`

## Ubuntu LTS (24.04+)

### Build dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
  cmake \
  ninja-build \
  g++ \
  pkg-config \
  qt6-base-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools
```

Then run the common source flow above.

## Fedora stable

### Build dependencies

```bash
sudo dnf install -y \
  cmake \
  ninja-build \
  gcc-c++ \
  pkgconf-pkg-config \
  qt6-qtbase-devel \
  qt6-qttools-devel
```

Then run the common source flow above.

## Validation references

- Distro plan: `docs/distro-validation-plan.md`
- RC1 baseline report: `docs/distro-validation-report-rc1-2026-02-13.md`
- Build details and variants: `BUILD.md`
