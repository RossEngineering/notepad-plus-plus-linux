# Build Guide

This fork is Linux-native first.
Primary reference environment: Arch Linux and derivatives (Manjaro baseline).

## Linux Quick Start (Arch-family, Manjaro baseline)

1. Install required packages:

```bash
sudo pacman -S --needed base-devel cmake ninja gcc pkgconf qt6-base qt6-tools
```

2. Configure:

```bash
cmake --preset debug
```

3. Build:

```bash
cmake --build --preset debug -- -j"$(nproc)"
```

4. Test:

```bash
ctest --preset debug --output-on-failure
```

5. Run:

```bash
./build/bin/notepad-plus-plus-linux
```

## Linux Build Variants

### Release build

```bash
cmake --preset release
cmake --build --preset release -- -j"$(nproc)"
ctest --preset release --output-on-failure
```

### Sanitizer build (memory and undefined behavior checks)

```bash
cmake -S . -B build-asan -G Ninja \
  -DBUILD_TESTING=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DNPP_BUILD_QT_SHELL=OFF \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined" \
  -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=address,undefined"

cmake --build build-asan -- -j"$(nproc)"
ASAN_OPTIONS="detect_leaks=0:halt_on_error=1" \
UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1" \
ctest --test-dir build-asan --output-on-failure
```

## Linux Packaging (Arch Linux and derivatives)

Use the repository `PKGBUILD`:

```bash
makepkg -si
```

Related docs:

- `docs/package-split-strategy.md`
- `docs/reproducible-release.md`
- `docs/release-publishing.md`

## Key CMake Targets

- `npp_scintilla`
- `npp_lexilla`
- `npp_platform_linux`
- `npp_linux_shell`
- `lexilla_smoke_test`
- `syntax_highlighting_smoke_test`
- `scintilla_document_smoke_test`
- `core_text_undo_redo_test`
- `encoding_large_file_regression_test`
- `platform_linux_services_smoke_test`
- `startup_typing_benchmark`

## Troubleshooting

- If `cmake` is not found in `zsh`, verify command correction is disabled for that call (`nocorrect cmake --version`).
- If Qt is missing, install `qt6-base` and `qt6-tools`.
- If tests fail due to local sanitizer tooling constraints, re-run without leak detection locally and rely on CI sanitizer job for leak gating.

## Windows Legacy Build Paths (Transition Period)

Windows build paths remain available while migration is in progress:

- Visual Studio solution: `PowerEditor/visual.net/notepadPlus.sln`
- MinGW path: `PowerEditor/gcc`

These are not the primary target for this fork, but remain intact to avoid regressions during transition.
