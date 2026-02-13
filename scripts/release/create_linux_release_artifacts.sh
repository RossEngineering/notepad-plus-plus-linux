#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VERSION="${1:-$(git -C "$ROOT_DIR" describe --tags --always --dirty)}"
OUTPUT_DIR="${2:-$ROOT_DIR/out/release}"
BUILD_DIR="$ROOT_DIR/build/release-artifacts"
STAGE_DIR="$OUTPUT_DIR/stage"

export LANG="C.UTF-8"
export LC_ALL="C.UTF-8"
export TZ="UTC"
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(git -C "$ROOT_DIR" log -1 --pretty=%ct)}"

rm -rf "$BUILD_DIR" "$STAGE_DIR"
mkdir -p "$OUTPUT_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_SKIP_INSTALL_RPATH=ON \
  -DBUILD_TESTING=OFF

cmake --build "$BUILD_DIR"
DESTDIR="$STAGE_DIR" cmake --install "$BUILD_DIR"

ARTIFACT_BASE="notepad-plus-plus-linux-${VERSION}-x86_64"
TARBALL="$OUTPUT_DIR/${ARTIFACT_BASE}.tar.xz"

# Create deterministic archive from staged install tree.
tar \
  --sort=name \
  --mtime="@${SOURCE_DATE_EPOCH}" \
  --owner=0 --group=0 --numeric-owner \
  -C "$STAGE_DIR" -cJf "$TARBALL" .

(
  cd "$OUTPUT_DIR"
  sha256sum "${ARTIFACT_BASE}.tar.xz" > "${ARTIFACT_BASE}.sha256"
)

echo "Created artifacts:"
echo "  $TARBALL"
echo "  $OUTPUT_DIR/${ARTIFACT_BASE}.sha256"
