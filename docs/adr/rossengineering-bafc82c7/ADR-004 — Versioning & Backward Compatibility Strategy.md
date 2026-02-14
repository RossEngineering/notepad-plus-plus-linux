# ADR-004 — Versioning & Backward Compatibility Strategy

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

**Status**: Accepted
**Date**: 2026-02-01
**Scope**: Organisation-wide (RossEngineering)

## Context

Projects within RossEngineering include APIs, platforms, and libraries that may evolve over time and potentially be consumed by others.

Unclear or casual versioning undermines trust and makes evolution painful. A principled stance on versioning, compatibility, and deprecation was required.

## Decision

RossEngineering adopts a conservative, respect-based approach to versioning.

### Core principles

- Backward compatibility is valued and preserved wherever reasonably possible.

- Breaking changes should not be introduced casually or for convenience.

- Deprecation should occur only when it is the economically soundest choice.

- Error formats and failure semantics should be as consistent as possible across the organisation.

### Practical guidance

- Semantic Versioning is preferred where versions are exposed.

- APIs should favour additive change over modification or removal.

- Deprecated behaviour should be:
  - clearly documented,
  - retained for a reasonable period,
  - and removed only with justification.

## Rationale

Versioning is a social contract.

Preserving compatibility:

- demonstrates respect for users and clients,

- reduces upgrade friction,

- and encourages adoption.

Breaking changes are sometimes necessary, but should be treated as costly decisions, not routine refactors.

## Consequences

### Positive

- Increased confidence for consumers of APIs and libraries

- Safer evolution of systems over time

- Stronger internal discipline around change

### Trade-offs

- Slower ability to “clean up” mistakes

- Additional maintenance burden for deprecated paths

## Notes

This ADR does not mandate:

- immediate versioning for early-stage projects,

- or premature public stability guarantees.

It **does** mandate intentionality once versions are published or behaviour is relied upon.
