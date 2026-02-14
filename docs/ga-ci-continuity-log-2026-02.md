# GA CI Continuity Log - 2026-02

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This log tracks the `v1.0.0` gate requiring required Linux lanes to remain green
for 7 consecutive days.

## Required lanes

1. `Linux CMake Build`
2. `Linux Desktop Integration`
3. `Win32 Boundary Guard`

## Continuity record

| Date (UTC) | Candidate commit | Linux CMake Build | Linux Desktop Integration | Win32 Boundary Guard | Day result | Evidence |
| --- | --- | --- | --- | --- | --- | --- |
| 2026-02-14 | `ae154f56c` | Pass | Pass | Pass | Counted (1/7) | `22016250682`, `22016250672`, `22016250679` |
| 2026-02-14 | `25214be78` | In progress | In progress | Pass | Pending | `22016385005`, `22016385013`, `22016385012` |

## Notes

1. Only days where all required lanes are green count toward the 7-day gate.
2. If any required lane fails on a counted day, continuity resets to 0 and a
   new 7-day window starts.
3. Keep this log updated daily until the gate is closed in
   `docs/releases/v1.0.0-checklist.md`.
