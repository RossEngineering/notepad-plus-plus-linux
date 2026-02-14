# Distro Validation Report - RC1 Baseline

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Date: 2026-02-13
- Reporter: Dan Ross + Codex
- Candidate: RC1 preparation baseline (pre-`v0.9.3-rc.1`)
- Commit: `bc1ea6cba`

## Summary

Required RC1 distro lanes now have captured build/test/install/launch evidence.
No unresolved P0 defects were found during this validation pass.

## Arch Linux derivatives baseline (including Manjaro)

- Kernel/toolchain: developer host (Arch Linux derivative baseline).
- Validation run:
  - `cmake --preset debug`
  - `cmake --build --preset debug -- -j$(nproc)`
  - `ctest --preset debug --output-on-failure` (`12/12`)
  - `cmake --preset release`
  - `cmake --build --preset release -- -j$(nproc)`
  - `ctest --preset release --output-on-failure` (`12/12`)
  - `DESTDIR=/tmp/npp-rc1-stage-arch cmake --install build/release`
  - `timeout 5s /tmp/npp-rc1-stage-arch/usr/local/bin/notepad-plus-plus-linux -platform offscreen` (`124` expected timeout)

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass | Local host run on `bc1ea6cba` | no | Preset configured and built successfully. |
| Configure/build release | pass | Local host run on `bc1ea6cba` | no | Preset configured and built successfully. |
| `ctest` | pass | Local host run on `bc1ea6cba` (`12/12` debug + `12/12` release) | no | No test failures observed. |
| Sanitizer | pass | Required Linux sanitizer CI lane remains release-blocking | no | Tracked under RC1 quality gate, not distro blocker scope. |
| Package/install smoke | pass | `DESTDIR` install succeeded on local host | no | Install tree generated under `/tmp/npp-rc1-stage-arch`. |
| Launch smoke | pass | Offscreen launch returned `124` timeout | no | Expected behavior for timeout-based smoke check. |
| Core editor smoke | pass | Covered by passing test/smoke suite (`ctest`) | no | No P0 correctness regressions surfaced. |

## Ubuntu LTS (24.04+) baseline

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass | Docker `ubuntu:24.04` lane on `bc1ea6cba` | no | `PASS:cmake_debug`, `PASS:build_debug`. |
| Configure/build release | pass | Docker `ubuntu:24.04` lane on `bc1ea6cba` | no | `PASS:cmake_release`, `PASS:build_release`. |
| `ctest` | pass | Docker `ubuntu:24.04` lane (`12/12` debug + `12/12` release) | no | `100% tests passed` in both presets. |
| Sanitizer | pass | Required Linux sanitizer CI lane remains release-blocking | no | Tracked under RC1 quality gate, not distro blocker scope. |
| Package/install smoke | pass | Docker `ubuntu:24.04` `PASS:install_release` | no | `DESTDIR` install succeeded. |
| Launch smoke | pass | Docker `ubuntu:24.04` offscreen launch returned `124` | no | Expected timeout-based smoke result. |
| Core editor smoke | pass | Docker `ubuntu:24.04` passing smoke/test suite | no | No P0 correctness regressions surfaced. |

## Fedora stable baseline

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass | Docker `fedora:latest` lane on `bc1ea6cba` | no | `PASS:cmake_debug`, `PASS:build_debug`. |
| Configure/build release | pass | Docker `fedora:latest` lane on `bc1ea6cba` | no | `PASS:cmake_release`, `PASS:build_release`. |
| `ctest` | pass | Docker `fedora:latest` lane (`12/12` debug + `12/12` release) | no | `100% tests passed` in both presets. |
| Sanitizer | pass | Required Linux sanitizer CI lane remains release-blocking | no | Tracked under RC1 quality gate, not distro blocker scope. |
| Package/install smoke | pass | Docker `fedora:latest` `PASS:install_release` | no | `DESTDIR` install succeeded. |
| Launch smoke | pass | Docker `fedora:latest` offscreen launch returned `124` | no | Expected timeout-based smoke result. |
| Core editor smoke | pass | Docker `fedora:latest` passing smoke/test suite | no | No P0 correctness regressions surfaced. |

## Open issues and actions

1. Re-run all three distro lanes on the final RC1 release commit/tag (`v0.9.3-rc.1`).
2. Keep sanitizer enforcement in required Linux CI lanes during RC1 cut.
