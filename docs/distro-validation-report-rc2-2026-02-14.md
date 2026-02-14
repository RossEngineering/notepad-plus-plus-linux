# Distro Validation Report - RC2 Candidate

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Date: 2026-02-14
- Reporter: Dan Ross + Codex
- Candidate: `v0.9.6-rc.2`
- Commit: `517d3324a`

## Summary

RC2 candidate validation was rerun on the Arch-derivatives baseline with full local quality gates and install/launch smoke.
Ubuntu/Fedora validation remains covered by required GitHub CI lanes for the final release candidate commit.

## Arch Linux derivatives baseline (including Manjaro)

- Validation run:
  - `ctest --preset debug --output-on-failure` (`13/13`)
  - `ctest --preset release --output-on-failure` (`13/13`)
  - `./scripts/check_win32_boundaries.sh HEAD~1` (pass)
  - `./scripts/check_startup_typing_budget.sh ./build/release/bin/startup_typing_benchmark --quick` (pass)
  - `DESTDIR=/tmp/npp-rc2-stage-arch cmake --install build/release`
  - `timeout 5s /tmp/npp-rc2-stage-arch/usr/local/bin/notepad-plus-plus-linux -platform offscreen` (`124` expected timeout)

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass | local host run for RC2 candidate | no | Existing preset build remains healthy. |
| Configure/build release | pass | local host run for RC2 candidate | no | Existing preset build remains healthy. |
| `ctest` | pass | local host run (`13/13` debug + `13/13` release) | no | New `lsp_baseline_ux_regression_test` included. |
| Performance budget gate | pass | `check_startup_typing_budget.sh` | no | RC2 CI budget thresholds satisfied locally. |
| Package/install smoke | pass | `DESTDIR` install succeeded | no | Install tree generated under `/tmp/npp-rc2-stage-arch`. |
| Launch smoke | pass | offscreen launch returned `124` timeout | no | Expected behavior for timeout-based smoke check. |

## Ubuntu LTS / Fedora stable

| Lane | Status | Evidence Source | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Ubuntu LTS | pending final tag validation | GitHub required Linux CI lanes | yes | Must be green on RC2 release/tag commit. |
| Fedora stable | pending final tag validation | GitHub required Linux CI lanes | yes | Must be green on RC2 release/tag commit. |

## Open actions

1. Confirm required GitHub lanes are green on `v0.9.6-rc.2` tag commit.
2. Attach lane links/check evidence to release notes/checklist.
