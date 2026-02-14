# Arch-Family Dev Setup (Manjaro Baseline)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This is the fastest path to a working local development environment for this repo.

## One-command setup

From repository root:

```bash
./scripts/dev-setup-manjaro.sh
```

The script will:

1. Install required packages with `pacman`.
2. Configure CMake using the `debug` preset.
3. Build with Ninja.
4. Run the test suite.

## Manual fallback

If you prefer not to run the helper script:

```bash
sudo pacman -S --needed base-devel cmake ninja gcc pkgconf qt6-base qt6-tools
cmake --preset debug
cmake --build --preset debug -- -j"$(nproc)"
ctest --preset debug --output-on-failure
```

## Run the app

```bash
./build/bin/notepad-plus-plus-linux
```

## Notes

- Target distro family for this workflow is Arch Linux and derivatives, with Manjaro as the baseline.
- For release packaging, see `PKGBUILD` and `docs/reproducible-release.md`.
