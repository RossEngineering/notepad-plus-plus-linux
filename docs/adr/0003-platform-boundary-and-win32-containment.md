# ADR 0003: Platform Boundary and Win32 Containment

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `docs/architecture.md`, `docs/coding-rules.md`

## Context

The legacy code path contains extensive direct Win32 usage. Linux-native work requires
clear service boundaries to avoid further coupling.

## Decision

Constrain OS-specific behavior behind a platform service layer and prevent new direct Win32
calls outside approved boundaries.

## Consequences

### Positive

- Reduces platform coupling and migration risk.
- Supports incremental replacement of Win32-dependent behavior.

### Negative

- Requires adapter/service implementation effort.
- Transitional dual-path complexity remains during migration.

## Alternatives Considered

1. Continue direct Win32 calls while porting.
2. Big-bang full rewrite before boundary work.

## Implementation Notes

- Linux service implementations added for path/filesystem/process/diagnostics.

## Evidence

- `platform/include/`
- `platform/linux/`
- `docs/architecture.md`
- `docs/coding-rules.md`
