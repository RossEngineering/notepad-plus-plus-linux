# ADR 0001: Linux-Native Target and Behavioral Compatibility

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `docs/compatibility-target.md`

## Context

This fork aims to deliver a native Linux editor experience for Arch Linux derivatives (including Manjaro as baseline) while starting
from a Windows-centric Notepad++ codebase.

## Decision

Target Linux-native execution and behavioral compatibility for MVP, not binary/ABI
compatibility with Windows builds.

## Consequences

### Positive

- Focuses effort on stable Linux behavior.
- Avoids fragile Win32 emulation paths.

### Negative

- No direct compatibility with Windows plugin binaries.
- Some UI behavior may differ from upstream Windows implementation.

## Alternatives Considered

1. Preserve Windows ABI compatibility in MVP.
2. Require Wine or compatibility layers.

## Implementation Notes

- Linux-first MVP scope in `docs/compatibility-target.md`.

## Evidence

- `docs/compatibility-target.md`
