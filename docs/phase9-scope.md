# Phase 9 Scope Brainstorm

Date: 2026-02-13

This document proposes post-Phase-8 priorities for the next implementation cycle.

## Goal

Shift from foundation/migration setup to alpha hardening and user-visible Linux parity.

## Candidate workstreams

## 1. Core reliability hardening

1. Add session restore regression suite (multi-tab + dirty state + crash-recovery path).
2. Add file I/O stress tests (large files, encoding edge cases, line-ending transitions).
3. Expand undo/redo coverage to grouped operations across commands (search/replace, multi-caret-ready paths).

## 2. UI parity and usability

1. Fill highest-impact Linux shell gaps versus current MVP workflows.
2. Improve keyboard shortcut parity and configurable keymap behavior.
3. Add higher-fidelity dialog behavior for find/replace/preferences edge cases.

## 3. Performance and startup optimization

1. Integrate benchmark trend checks into CI artifacts.
2. Add regression thresholds for startup and typing metrics.
3. Profile first-open path and reduce initialization overhead.

## 4. Distribution and release hardening

1. Validate package install/upgrade/uninstall on clean Manjaro snapshots.
2. Add reproducibility verification job (checksum stability check across clean runs).
3. Add release checklist gating to prevent unsigned/incomplete artifacts.

## 5. Plugin and extension strategy execution

1. Convert plugin strategy from decision-only to initial API draft.
2. Build minimal plugin-host proof-of-concept for Linux-native extension loading.
3. Document compatibility expectations for future contributors.

## Proposed Phase 9 exit criteria

1. No open `priority:p0` regressions in Linux CI scope.
2. Expanded regression suite covers session, encoding, and UI-critical paths.
3. Performance baseline checks are automated and visible in CI.
4. Release pipeline passes with verified artifacts and documented checklist.
5. Phase 10 scope is defined from measured Phase 9 outcomes.
