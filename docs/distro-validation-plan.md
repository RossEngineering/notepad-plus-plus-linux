# Distro Validation Expansion Plan

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14

## Scope

Phase 12 expands validation from the Arch Linux derivative baseline to a broader Linux distro matrix while preserving Arch-family first-class support.

## Matrix

Primary:

1. Arch Linux / derivatives (including Manjaro baseline)

Expansion:

1. Ubuntu LTS (24.04+)
2. Debian stable
3. Fedora current stable

## Validation lanes

Per distro lane:

1. Configure and build (debug + release)
2. Core regression tests (`ctest`)
3. Sanitizer lane where supported
4. Package/install smoke check
5. App launch smoke check
6. Wayland compositor launch smoke check (headless `weston` + `QT_QPA_PLATFORM=wayland`)

## Rollout plan

1. Keep Arch-family as release-blocking baseline.
2. Add Ubuntu lane as first non-Arch required lane.
3. Add Debian and Fedora as non-blocking informational lanes.
4. Promote Debian and Fedora to required once flake rate is controlled.

## Exit criteria

1. Arch + Ubuntu lanes stable and required.
2. Debian + Fedora lanes stable for 14 consecutive days.
3. Known distro-specific issues documented with owner and mitigation path.
