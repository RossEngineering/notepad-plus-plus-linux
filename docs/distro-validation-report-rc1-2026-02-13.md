# Distro Validation Report - RC1 Bootstrap

- Date: 2026-02-13
- Reporter: Dan Ross + Codex
- Candidate: RC1 preparation baseline (pre-`v0.9.3-rc.1`)
- Commit: `a5eea79ef`

## Summary

This is the first published RC1 validation report using the RC evidence format.
It captures current baseline status and remaining validation work for required RC1 distro gates.

## Arch Linux derivatives baseline (including Manjaro)

- Kernel/toolchain: local developer environment baseline.
- Current confidence: medium-high from active development usage and prior beta release validation.

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pass | Local RC work + Beta 2 release evidence | yes | Re-run on RC1 candidate SHA required. |
| Configure/build release | pass | Local RC work + Beta 2 release evidence | yes | Re-run on RC1 candidate SHA required. |
| `ctest` | pass | Beta 2 gate evidence (`11/11`) | yes | Re-run on RC1 candidate SHA required. |
| Sanitizer | pass | Prior required lane evidence | yes | Re-run on RC1 candidate SHA required. |
| Package/install smoke | pass | Beta 2 artifact/release process | yes | Validate with RC1 artifact names/sha. |
| Launch smoke | pass | Prior beta smoke runs | yes | Reconfirm on RC1 candidate build. |
| Core editor smoke | pass | Beta 2 stabilization evidence | yes | Reconfirm on RC1 candidate build. |

## Ubuntu LTS (24.04+) baseline

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Configure/build release | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| `ctest` | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Sanitizer | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Package/install smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Launch smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Core editor smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |

## Fedora stable baseline

| Check | Result | Evidence | Blocking | Notes |
| --- | --- | --- | --- | --- |
| Configure/build debug | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Configure/build release | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| `ctest` | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Sanitizer | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Package/install smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Launch smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |
| Core editor smoke | pending | Not yet executed for RC1 report | yes | Run on RC1 candidate SHA. |

## Open issues and actions

1. Execute Ubuntu RC1 validation lane and capture evidence.
2. Execute Fedora RC1 validation lane and capture evidence.
3. Re-run Arch derivatives checks on final RC1 candidate SHA.
