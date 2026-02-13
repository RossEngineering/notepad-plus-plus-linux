# Architecture Decision Records (ADR)

This folder tracks architecture decisions for `notepad-plus-plus-linux`.

## Purpose

ADRs capture why significant technical choices were made, the alternatives considered,
and what tradeoffs were accepted.

## Naming and numbering

- Use zero-padded sequence numbers: `0000`, `0001`, `0002`, ...
- Filename pattern: `<number>-<short-kebab-title>.md`
- Never renumber existing ADRs.

## Status values

- `Proposed`
- `Accepted`
- `Superseded` (include pointer to newer ADR)
- `Deprecated`

## When to write an ADR

Create or update an ADR when a decision affects at least one of:

- architecture boundaries or layering
- platform/toolkit strategy
- packaging/distribution model
- plugin/runtime compatibility model
- release and operational practices

## Workflow

1. Copy `0000-adr-template.md`.
2. Fill Context, Decision, Consequences, and Alternatives.
3. Link implementation evidence (code/docs/workflows).
4. Set status (`Proposed` or `Accepted`).
5. If replacing an old ADR, mark old ADR as `Superseded`.
