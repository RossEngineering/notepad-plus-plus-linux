# GA CI Continuity Log - 2026-02

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

This log tracks required Linux-lane continuity evidence during `v1.0.0` closeout.

As of 2026-02-14, this continuity threshold is informational (non-blocking) for
`v1.0.0` under `docs/ga-gate-exception-2026-02-14.md`.

## Required lanes

1. `Linux CMake Build`
2. `Linux Desktop Integration`
3. `Win32 Boundary Guard`

## Daily update flow

1. Ensure GitHub CLI auth is ready (`gh auth login` if not already authenticated).
2. Wait for the target commit's required CI lanes to finish.
3. Run:
   - `./scripts/ga/update_ci_continuity_log.sh --commit <sha>`
4. Commit the continuity log update on `ga-dev-week1` (or current GA tracking branch).
5. If needed, preview without editing using:
   - `./scripts/ga/update_ci_continuity_log.sh --commit <sha> --dry-run`

## Continuity record

| Date (UTC) | Candidate commit | Linux CMake Build | Linux Desktop Integration | Win32 Boundary Guard | Day result | Evidence |
| --- | --- | --- | --- | --- | --- | --- |
| 2026-02-14 | `ae154f56c` | Pass | Pass | Pass | Counted (1/7) | `22016250682`, `22016250672`, `22016250679` |
| 2026-02-14 | `25214be78` | Pass | Pass | Pass | Pass (same day, continuity unchanged) | `22016385005`, `22016385013`, `22016385012` |
| 2026-02-14 | `74c2e7e98` | Pass | Pass | Pass | Pass (same day, continuity unchanged) | `22016423969`, `22016423986`, `22016423973` |
| 2026-02-14 | `14b73c212` | Pass | Pass | Pass | Pass (same day, continuity unchanged) | `22016991517`, `22016991516`, `22016991524` |
| 2026-02-14 | `41d844aa9` | Pass | Pass | Pass | Pass (same day, continuity unchanged) | `22022023357`, `22022023363`, `22022023361` |

## Notes

1. Only days where all required lanes are green count toward the continuity
   streak.
2. For `v1.0.0`, this streak is informational (exception-approved), not a hard
   release blocker.
3. If any required lane fails on a counted day, continuity resets to 0 and a
   new window starts.
4. Multiple green commits on the same UTC day do not increment the continuity
   counter beyond that day's single count.
5. Keep this log updated daily until GA cut, then retain it as post-GA
   monitoring evidence.
