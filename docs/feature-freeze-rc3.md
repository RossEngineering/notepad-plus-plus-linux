# RC3 Feature Freeze Policy

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14
Applies to: `v0.9.7` through `v0.9.9-rc.3`

## Goal

RC3 is a release-readiness phase. The objective is to reduce risk and maximize
confidence before `v1.0.0`.

## Allowed change types

1. Bug fixes (correctness, crash, data-loss, recovery, security).
2. Packaging/install reliability improvements.
3. CI/release engineering reliability updates.
4. Documentation and release-note updates.
5. Test additions or test hardening tied to RC3 blockers.

## Disallowed change types

1. Net-new end-user features that are not required for RC3 blockers.
2. Large refactors without direct release-risk reduction.
3. New dependencies without explicit release-blocker justification.
4. UI redesign work not required for correctness or release reliability.

## Exception process

1. Open an issue tagged `rc3-freeze-exception`.
2. Document:
   - risk addressed
   - scope and rollback plan
   - why deferring to post-`v0.9.9-rc.3` is unacceptable
3. Link the issue in the PR before merge.

## Merge gate during freeze

1. Every PR must state one of:
   - `freeze-type: bugfix`
   - `freeze-type: packaging`
   - `freeze-type: release`
   - `freeze-type: docs`
   - `freeze-type: tests`
2. PR description must include regression/test evidence.
3. Required Linux CI lanes must remain green.

## Related docs

- `docs/roadmap.md`
- `docs/rc3-blockers.md`
- `docs/releases/v0.9.9-rc.3-checklist.md`
