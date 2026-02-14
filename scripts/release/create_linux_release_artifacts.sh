#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
VERSION="${1:-$(git -C "$ROOT_DIR" describe --tags --always --dirty)}"
OUTPUT_DIR="${2:-$ROOT_DIR/out/release}"
BUILD_DIR="${NPP_RELEASE_BUILD_DIR:-$ROOT_DIR/build/release-artifacts}"
STAGE_DIR="${NPP_RELEASE_STAGE_DIR:-$OUTPUT_DIR/stage}"
NATIVE_PACKAGE_OUTPUTS="${NPP_RELEASE_NATIVE_PACKAGES:-1}"

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

require_cmd() {
  local cmd="$1"
  if ! command -v "$cmd" >/dev/null 2>&1; then
    echo "release artifact staging failed: required command not found: ${cmd}" >&2
    exit 1
  fi
}

sanitize_for_package_component() {
  local value="$1"
  value="${value//[^A-Za-z0-9.+~_-]/.}"
  value="${value#.}"
  value="${value%.}"
  if [[ -z "${value}" ]]; then
    value="0"
  fi
  printf '%s' "${value}"
}

raw_version="${VERSION#v}"
if [[ -z "${raw_version}" ]]; then
  raw_version="${VERSION}"
fi
raw_version="$(sanitize_for_package_component "${raw_version}")"
raw_version="${raw_version#v}"
if [[ -z "${raw_version}" ]]; then
  raw_version="0.0.0"
fi

debian_version="${raw_version}"
debian_version="${debian_version/-beta./~beta.}"
debian_version="${debian_version/-rc./~rc.}"
debian_version="${debian_version//-/.}"
debian_version="$(sanitize_for_package_component "${debian_version}")"

rpm_version="${raw_version%%-*}"
rpm_version="$(sanitize_for_package_component "${rpm_version}")"
if [[ -z "${rpm_version}" ]]; then
  rpm_version="0.0.0"
fi
rpm_release="1"
if [[ "${raw_version}" == *-* ]]; then
  rpm_suffix="${raw_version#*-}"
  rpm_suffix="$(sanitize_for_package_component "${rpm_suffix//-/.}")"
  rpm_release="0.${rpm_suffix}.1"
fi

arch_pkgver="${raw_version//-/_}"
arch_pkgver="$(sanitize_for_package_component "${arch_pkgver}")"
arch_pkgrel="1"
arch_pkgver_full="${arch_pkgver}-${arch_pkgrel}"

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
ARCH_PKG="$OUTPUT_DIR/notepad-plus-plus-linux-${arch_pkgver}-${arch_pkgrel}-x86_64.pkg.tar.zst"
DEB_PKG="$OUTPUT_DIR/notepad-plus-plus-linux_${debian_version}_amd64.deb"
RPM_PKG_BASENAME="notepad-plus-plus-linux-${rpm_version}-${rpm_release}.x86_64.rpm"
RPM_PKG="$OUTPUT_DIR/${RPM_PKG_BASENAME}"

require_cmd tar
require_cmd xz

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

if [[ "${NATIVE_PACKAGE_OUTPUTS}" == "1" ]]; then
  require_cmd zstd
  require_cmd dpkg-deb
  require_cmd rpmbuild

  arch_package_root="$(mktemp -d)"
  deb_package_root="$(mktemp -d)"
  rpm_topdir="$(mktemp -d)"
  cleanup_packaging_dirs() {
    rm -rf "$arch_package_root" "$deb_package_root" "$rpm_topdir"
  }
  trap cleanup_packaging_dirs EXIT

  # Create deterministic Arch package (.pkg.tar.zst).
  mkdir -p "${arch_package_root}"
  cp -a "${STAGE_DIR}/usr" "${arch_package_root}/"
  install_script="${ROOT_DIR}/packaging/arch/notepad-plus-plus-linux.install"
  if [[ -f "${install_script}" ]]; then
    cp "${install_script}" "${arch_package_root}/.INSTALL"
  fi
  installed_size="$(du -sb "${arch_package_root}/usr" | awk '{print $1}')"
  cat > "${arch_package_root}/.PKGINFO" <<EOF
pkgname = notepad-plus-plus-linux
pkgbase = notepad-plus-plus-linux
pkgver = ${arch_pkgver_full}
pkgdesc = Linux-native Notepad++ fork for Arch Linux and derivatives
url = https://github.com/RossEngineering/notepad-plus-plus-linux
builddate = ${SOURCE_DATE_EPOCH}
packager = Dan Ross <danross2683@gmail.com>
size = ${installed_size}
arch = x86_64
license = GPL-3.0-or-later
depend = qt6-base
depend = qt6-5compat
depend = glibc
depend = gcc-libs
EOF

  arch_entries=(
    ".PKGINFO"
    "usr"
  )
  if [[ -f "${arch_package_root}/.INSTALL" ]]; then
    arch_entries=(
      ".PKGINFO"
      ".INSTALL"
      "usr"
    )
  fi

  (
    cd "${arch_package_root}"
    tar \
      --sort=name \
      --mtime="@${SOURCE_DATE_EPOCH}" \
      --owner=0 --group=0 --numeric-owner \
      --zstd -cf "${ARCH_PKG}" \
      "${arch_entries[@]}"
  )

  # Create Debian package (.deb).
  mkdir -p "${deb_package_root}/DEBIAN"
  cp -a "${STAGE_DIR}/usr" "${deb_package_root}/"
  cat > "${deb_package_root}/DEBIAN/control" <<EOF
Package: notepad-plus-plus-linux
Version: ${debian_version}
Section: editors
Priority: optional
Architecture: amd64
Maintainer: Dan Ross <danross2683@gmail.com>
Depends: libc6, libgcc-s1, libstdc++6, libqt6core6t64 | libqt6core6, libqt6gui6t64 | libqt6gui6, libqt6widgets6t64 | libqt6widgets6, libqt6opengl6t64 | libqt6opengl6, libqt6openglwidgets6t64 | libqt6openglwidgets6, libqt65compat6
Description: Linux-native Notepad++ fork
 Notepad++ Linux is a Linux-native editor inspired by Notepad++ workflows.
 This package installs desktop integration, MIME mapping, and built-in skins.
EOF
  cat > "${deb_package_root}/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q /usr/share/applications || true
fi
if command -v update-mime-database >/dev/null 2>&1; then
  update-mime-database /usr/share/mime || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -qtf /usr/share/icons/hicolor || true
elif command -v gtk4-update-icon-cache >/dev/null 2>&1; then
  gtk4-update-icon-cache -qtf /usr/share/icons/hicolor || true
fi
EOF
  cat > "${deb_package_root}/DEBIAN/postrm" <<'EOF'
#!/bin/sh
set -e
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q /usr/share/applications || true
fi
if command -v update-mime-database >/dev/null 2>&1; then
  update-mime-database /usr/share/mime || true
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -qtf /usr/share/icons/hicolor || true
elif command -v gtk4-update-icon-cache >/dev/null 2>&1; then
  gtk4-update-icon-cache -qtf /usr/share/icons/hicolor || true
fi
EOF
  chmod 0755 "${deb_package_root}/DEBIAN/postinst" "${deb_package_root}/DEBIAN/postrm"
  dpkg-deb --build --root-owner-group "${deb_package_root}" "${DEB_PKG}"

  # Create RPM package (.rpm).
  mkdir -p \
    "${rpm_topdir}/BUILD" \
    "${rpm_topdir}/BUILDROOT" \
    "${rpm_topdir}/RPMS" \
    "${rpm_topdir}/SOURCES" \
    "${rpm_topdir}/SPECS" \
    "${rpm_topdir}/SRPMS"

  cat > "${rpm_topdir}/SPECS/notepad-plus-plus-linux.spec" <<EOF
Name:           notepad-plus-plus-linux
Version:        ${rpm_version}
Release:        ${rpm_release}
Summary:        Linux-native Notepad++ fork
License:        GPL-3.0-or-later
URL:            https://github.com/RossEngineering/notepad-plus-plus-linux
BuildArch:      x86_64
Requires:       qt6-qtbase
Requires:       qt6-qt5compat

%description
Notepad++ Linux is a Linux-native editor inspired by Notepad++ workflows.

%prep

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr
cp -a ${STAGE_DIR}/usr/. %{buildroot}/usr/

%post
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q /usr/share/applications || :
fi
if command -v update-mime-database >/dev/null 2>&1; then
  update-mime-database /usr/share/mime || :
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -qtf /usr/share/icons/hicolor || :
elif command -v gtk4-update-icon-cache >/dev/null 2>&1; then
  gtk4-update-icon-cache -qtf /usr/share/icons/hicolor || :
fi

%postun
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database -q /usr/share/applications || :
fi
if command -v update-mime-database >/dev/null 2>&1; then
  update-mime-database /usr/share/mime || :
fi
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -qtf /usr/share/icons/hicolor || :
elif command -v gtk4-update-icon-cache >/dev/null 2>&1; then
  gtk4-update-icon-cache -qtf /usr/share/icons/hicolor || :
fi

%files
%defattr(-,root,root,-)
/usr/bin/notepad-plus-plus-linux
/usr/share/applications/notepad-plus-plus-linux.desktop
/usr/share/icons/hicolor/scalable/apps/notepad-plus-plus-linux.svg
/usr/share/mime/packages/notepad-plus-plus-linux.xml
/usr/share/notepad-plus-plus-linux/skins/*
/usr/share/notepad-plus-plus-linux/default-update-channel
/usr/share/notepad-plus-plus-linux/extensions/index.json
EOF

  rpmbuild \
    --define "_topdir ${rpm_topdir}" \
    --define "_build_id_links none" \
    -bb "${rpm_topdir}/SPECS/notepad-plus-plus-linux.spec"
  cp "${rpm_topdir}/RPMS/x86_64/${RPM_PKG_BASENAME}" "${RPM_PKG}"
fi

echo "Created artifacts:"
echo "  $TARBALL"
echo "  $OUTPUT_DIR/${ARTIFACT_BASE}.sha256"
if [[ "${NATIVE_PACKAGE_OUTPUTS}" == "1" ]]; then
  echo "  $ARCH_PKG"
  echo "  $DEB_PKG"
  echo "  $RPM_PKG"
fi
