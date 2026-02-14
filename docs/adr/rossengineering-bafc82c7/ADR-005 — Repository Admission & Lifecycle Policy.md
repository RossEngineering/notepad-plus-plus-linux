# ADR-005 — Repository Admission & Lifecycle Policy

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

**Status**: Accepted
**Date**: 2026-01-26
**Scope**: Organisation-wide (RossEngineering)

## Context

RossEngineering exists to house professional, portfolio-grade software engineering work.
Without clear admission criteria, organisations tend to accumulate unfinished, inconsistent, or misaligned repositories over time.

A decision was needed to define:

- the minimum quality bar for repositories

- whether repositories can move between personal and organisational ownership

- how to treat experimental or archived work

## Decision

All repositories admitted to RossEngineering must meet a defined professional baseline and are considered promoted, not stored.

### Admission requirements

- A structured, professional README.md

- Automated tests

- A CI pipeline

- A decision log (docs/decisions.md)

### Lifecycle rules

- Repositories may exist in an incubator state within the organisation

- Repositories do not return to personal accounts once promoted

- Archived repositories remain visible and meaningful

- Archived does not imply deleted or abandoned

## Rationale

Promotion into RossEngineering signals intent and credibility.
A clear bar prevents erosion of standards and ensures the organisation remains coherent over time.

An incubator state allows experimentation without lowering expectations, while a no-demotion rule preserves narrative integrity and accountability.

## Consequences

### Positive

- Clear quality signal to reviewers and collaborators

- Reduced long-term maintenance debt

- Strong separation between experimentation and professional work

### Trade-offs

- Slightly higher friction before promotion

- Requires discipline to maintain admission standards
