# Distro Validation Report - RC3 Integration Gate

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Date: 2026-02-14
- Reporter: Codex
- Candidate track: `v0.9.9-rc.3` (published)
- Branch: `master`

## Summary

Validated the consumer install/uninstall and file-handler integration flow end-to-end across all RC3 target distributions using the matrix harness.

## Validation commands

1. `./scripts/linux/run-desktop-integration-matrix.sh arch-derivative`
2. `./scripts/linux/run-desktop-integration-matrix.sh ubuntu-lts`
3. `./scripts/linux/run-desktop-integration-matrix.sh fedora-stable`

## Results

| Distro lane | Desktop integration (stage + installed) | File-handler defaults | Uninstall cleanup | Result |
| --- | --- | --- | --- | --- |
| Arch derivatives (including Manjaro) | pass | pass | pass | pass |
| Ubuntu LTS | pass | pass | pass | pass |
| Fedora stable | pass | pass | pass | pass |

## Evidence notes

- Each lane completed with `desktop/file-handler integration matrix validation passed for <lane>`.
- The harness also produced per-lane release artifacts and checksums under `out/release/`.
- Non-blocking desktop hints observed from `desktop-file-validate`:
  - `Categories` key contains more than one main category (`Utility;TextEditor;Development;IDE;`).

## RC3 impact

1. Closes RC3 file-handler integration gate in `TODO.md`.
2. Supports closing `RC3-B01` and `RC3-B02` in `docs/rc3-blockers.md`.
3. Provides GA promotion evidence for distro install and file-handler behavior.
