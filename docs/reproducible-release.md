# Reproducible Release Build (Linux)

Last updated: 2026-02-13
Target: Manjaro/Arch Linux

## Purpose

Produce release artifacts that are bit-for-bit reproducible from the same commit and toolchain.

## Preconditions

- Clean working tree (`git status` shows no pending changes).
- Fixed toolchain versions for CMake, compiler, Qt, and Ninja.
- Build in a clean container/chroot (recommended: clean Arch/Manjaro build root).

## Deterministic environment

Set these variables before configuring/building:

```bash
export LANG=C.UTF-8
export LC_ALL=C.UTF-8
export TZ=UTC
export SOURCE_DATE_EPOCH="$(git log -1 --pretty=%ct)"
```

## Build steps

```bash
cmake -S . -B build/release-repro -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_SKIP_INSTALL_RPATH=ON \
  -DBUILD_TESTING=OFF

cmake --build build/release-repro
DESTDIR="$PWD/out/stage" cmake --install build/release-repro
```

## Deterministic archive

```bash
mkdir -p out

tar \
  --sort=name \
  --mtime="@${SOURCE_DATE_EPOCH}" \
  --owner=0 --group=0 --numeric-owner \
  -C out/stage -cJf out/notepad-plus-plus-linux.tar.xz .
```

## Verification

Rebuild from the same commit in a second clean environment and compare:

```bash
sha256sum out/notepad-plus-plus-linux.tar.xz
```

Matching hash values indicate reproducible output.

## Notes

- Keep release builds on a pinned distro/toolchain image.
- If hashes differ, check locale, timestamps, RPATH, and dependency versions first.
