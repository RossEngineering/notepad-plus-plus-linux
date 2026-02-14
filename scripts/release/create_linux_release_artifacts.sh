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

MAX_SOURCE_DATE_EPOCH=253402300799
SOURCE_DATE_EPOCH_CANDIDATE="${SOURCE_DATE_EPOCH:-}"
if [[ -z "${SOURCE_DATE_EPOCH_CANDIDATE}" ]]; then
  SOURCE_DATE_EPOCH_CANDIDATE="$(git -C "$ROOT_DIR" log -1 --pretty=%ct 2>/dev/null || true)"
fi
if [[ ! "${SOURCE_DATE_EPOCH_CANDIDATE}" =~ ^[0-9]+$ ]] || \
   (( SOURCE_DATE_EPOCH_CANDIDATE < 0 || SOURCE_DATE_EPOCH_CANDIDATE > MAX_SOURCE_DATE_EPOCH )); then
  SOURCE_DATE_EPOCH_CANDIDATE="$(date +%s)"
fi
export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH_CANDIDATE}"

rm -rf "$BUILD_DIR" "$STAGE_DIR"
mkdir -p "$OUTPUT_DIR" "$STAGE_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_SKIP_INSTALL_RPATH=ON \
  -DBUILD_TESTING=OFF

cmake --build "$BUILD_DIR"
DESTDIR="$STAGE_DIR" cmake --install "$BUILD_DIR"

EXPECTED_BIN="$STAGE_DIR/usr/bin/notepad-plus-plus-linux"
if [[ ! -f "$EXPECTED_BIN" ]]; then
  echo "Release artifact staging failed: expected binary not found at $EXPECTED_BIN" >&2
  echo "This usually means Qt dependencies are missing and npp_linux_shell was skipped at configure time." >&2
  exit 1
fi

ARTIFACT_BASE="notepad-plus-plus-linux-${VERSION}-x86_64"
TARBALL="$OUTPUT_DIR/${ARTIFACT_BASE}.tar.xz"

if ! command -v xz >/dev/null 2>&1; then
  echo "release artifact staging failed: xz is required to create .tar.xz artifacts" >&2
  exit 1
fi

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
