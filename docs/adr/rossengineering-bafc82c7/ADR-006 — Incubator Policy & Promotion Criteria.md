# ADR-006 — Incubator Policy & Promotion Criteria

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

**Status**: Accepted
**Date**: 2026-02-01
**Scope**: Organisation-wide (RossEngineering)

## Context

RossEngineering is intended to represent **professional, portfolio-grade software engineering work**.
However, not all projects begin life in a fully formed state.

Some work requires:

- exploration,

- iteration,

- refactoring,

- or deliberate design experimentation,

before it can meet the organisation’s admission standards.

Without a formal mechanism to support this phase, projects risk either:

being promoted too early, or

remaining permanently unfinished outside the organisation.

An **Incubator** concept was required to bridge this gap.

## Decision

RossEngineering adopts an Incubator policy to support early-stage projects within the organisation, without lowering professional standards.

### Definition

An Incubator project is a repository that:

- is intended to become portfolio-grade,

- but has not yet met all admission requirements,

- and is undergoing deliberate development or restructuring.

Incubator status is **temporary by design**.

## Incubator Characteristics

An incubator repository:

- Lives inside RossEngineering

- Is clearly marked as Incubator in its README

- May have:
  - incomplete feature sets,
  - partial test coverage,
  - evolving architecture,
  - provisional documentation

However, it must still demonstrate **intentionality**.

## Minimum Requirements (Even in Incubator)

Even while incubating, a repository must have:

- A clear, honest README stating:
  - the project’s intent,
  - current status,
  - and known gaps

- A basic project structure

- Evidence of active reasoning (for example, early decision entries)

Incubator status does **not** excuse chaos or neglect.

## Promotion Criteria

A repository may graduate from Incubator status once it satisfies the **Repository Admission ADR**, including:

- A structured, professional README

- Automated tests

- CI pipeline (build + test)

- Decision log (docs/decisions.md)

Promotion is an **affirmative act**, not an automatic outcome of time passing.

## Lifecycle Rules

- Incubator repositories:
  - may evolve rapidly,
  - may change direction,
  - may be paused or abandoned

- If abandoned:
  - the repository may be archived,
  - with its incubator status retained for historical context

- Repositories do not return to personal accounts once incubated.

## Rationale

This policy recognises that:

- professional systems are grown, not created fully formed,

- early messiness is acceptable when paired with clarity of intent,

- and standards should be enforced at promotion, not experimentation.

It allows learning and iteration **without eroding organisational coherence**.

## Consequences

### Positive

- Encourages ambitious, thoughtful projects

- Prevents premature promotion

- Makes learning phases explicit and honest

- Preserves a clean organisational narrative

### Trade-offs

- Requires conscious review to promote or archive

- Adds a small amount of governance overhead

## Notes

Incubator status should be:

- visible,

- honest,

- and periodically revisited.

A project that remains incubated indefinitely should prompt a decision:
**promote**, **archive**, or **deliberately pause**.
