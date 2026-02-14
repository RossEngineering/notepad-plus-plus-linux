# GA Gate Exception - 2026-02-14

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

## Decision

For `v1.0.0` only, the 7-day CI continuity gate is vetoed and treated as
non-blocking.

## Rationale

1. External tester intake is currently limited.
2. Maintainer-led daily usage has not surfaced blocking defects.
3. Required Linux lanes are still enforced on the active GA candidate commit.

## Compensating controls

1. Keep required Linux workflows green on the selected GA tag-candidate commit:
   - `Linux CMake Build`
   - `Linux Desktop Integration`
   - `Win32 Boundary Guard`
2. Continue daily triage of incoming issues and escalate reproducible
   crash/data-loss/install defects as `P0` GA blockers.
3. Continue updating `docs/ga-ci-continuity-log-2026-02.md` as operational
   monitoring evidence (informational for `v1.0.0`).
4. If any `P0` regression appears during GA cut, defer tag and return to blocker
   closure flow.

## Scope and expiry

1. Applies only to `v1.0.0` promotion.
2. Does not remove CI continuity as a recommended quality signal for post-GA
   releases.
