# ADR-008 — Testing & Quality Assurance Expectations

**Status**: Accepted
**Date**: 2026-02-01
**Scope**: Organisation-wide (RossEngineering)

## Context

RossEngineering prioritises correctness and long-term maintainability.
Testing is a primary mechanism for enforcing these values, but expectations needed to be explicit to avoid ambiguity.

## Decision

Testing is a first-class requirement for all repositories.

### Core principles

- If behaviour can be tested, it should be tested

- It is preferable to fail during testing than in production or client contexts

- Tests may lag during early exploration, but must catch up regularly

- If a project builds and has no tests, this is considered a failure

Testing is expected to evolve alongside the system.

## Rationale

Tests provide evidence of correctness and confidence in change.
They are not optional polish, but part of the engineering work itself.

Allowing early lag acknowledges real-world iteration, while requiring eventual coverage prevents permanent test debt.

## Consequences

### Positive

- Increased confidence in changes

- Safer refactoring and evolution

- Clear quality baseline across repositories

### Trade-offs

- Slower initial velocity in some projects

- Ongoing maintenance effort for tests
