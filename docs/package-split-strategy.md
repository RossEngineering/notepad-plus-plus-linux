# Package Split Strategy (Arch Linux and derivatives)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-13
Status: Accepted for Phase 6

## Goals

- Keep the default install small for end users.
- Make debug symbols optional for diagnostics and crash analysis.
- Isolate plugin deliverables so plugin ABI work can evolve independently.

## Package layout

1. `notepad-plus-plus-linux` (runtime)
   - Main desktop application binary and required runtime assets.
   - Desktop entry, icon, MIME metadata.
   - This is the default package for users.

2. `notepad-plus-plus-linux-debug` (debug symbols)
   - Detached debug symbols for the main executable and future shared libs.
   - No runtime dependency for normal users.
   - Intended for developers and issue triage.

3. `notepad-plus-plus-linux-plugins` (optional)
   - Linux-native plugin bundles once plugin API stabilizes.
   - Explicitly optional and version-gated to host API compatibility.

## Build/packaging behavior

- Runtime package must be buildable standalone and remain the only mandatory package.
- Debug package should track the exact runtime build ID/version.
- Plugin package should depend on matching runtime major/minor API level.

## Near-term implementation notes

- Current `PKGBUILD` covers runtime package first.
- Split package outputs (`-debug`, `-plugins`) will be enabled incrementally as:
  1. reproducible release artifacts are finalized,
  2. plugin API surface moves from decision stage to implementation.
