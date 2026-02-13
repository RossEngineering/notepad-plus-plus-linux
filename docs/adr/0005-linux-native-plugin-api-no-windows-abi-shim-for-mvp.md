# ADR 0005: Linux-Native Plugin API, No Windows ABI Shim for MVP

- Status: Accepted
- Date: 2026-02-13
- Decision Makers: project maintainers
- Related: `docs/plugin-strategy.md`

## Context

Plugin compatibility is a major risk area because existing contracts are Win32- and DLL-centric.

## Decision

For MVP, choose Linux-native plugin API direction and explicitly do not implement a Windows ABI
compatibility shim.

## Consequences

### Positive

- Keeps MVP scope and stability manageable.
- Avoids fragile ABI translation work early.

### Negative

- Existing Windows plugins are not directly reusable.
- Native plugin API work remains ahead post-MVP.

## Alternatives Considered

1. Compatibility shim for Windows plugins.
2. Delay plugin strategy decision.

## Implementation Notes

- Documented as accepted project strategy.

## Evidence

- `docs/plugin-strategy.md`
