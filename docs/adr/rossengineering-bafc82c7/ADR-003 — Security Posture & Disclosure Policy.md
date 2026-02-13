# ADR-003 — Security Posture & Disclosure Policy

**Status**: Accepted
**Date**: 2026-02-01
**Scope**: Organisation-wide (RossEngineering)

## Context

RossEngineering hosts professional, portfolio-grade software projects with varying scopes, runtimes, and threat profiles.

Security is an important concern, but the organisation includes:

- learning-oriented systems,

- backend APIs,

- orchestration layers,

- and experimental platforms.

A single, rigid security policy would either be overbearing for small projects or insufficiently nuanced for larger ones. A balanced, principle-driven approach was required.

## Decision

RossEngineering adopts a graduated, project-appropriate security posture.

### Core decisions

- Security expectations are explicit but proportional to the project’s scope.

- Security issues should be reported privately, not via public issues.

- GitHub Security Advisories may be enabled per repository where appropriate.

- An organisation-wide SECURITY.md may be introduced later, but is not mandatory at this stage.

### Disclosure expectations

- Security concerns should be raised responsibly and with sufficient detail to reproduce.

- Public discussion of security issues should occur only after mitigation or acceptance of risk.

- Portfolio projects are not assumed to be production-hardened unless explicitly stated.

## Rationale

Security is not binary; it exists on a spectrum.

This approach:

- avoids performative security theatre,

- respects the learning and exploratory nature of some projects,

- while still setting clear expectations for responsible disclosure.

Explicit acknowledgement of security boundaries is preferable to silent assumptions.

## Consequences

### Positive

- Clear expectations without overcommitment

- Reduced risk of accidental public disclosure

- Flexibility to tighten posture as projects mature

### Trade-offs

- No single “security guarantee” across all repos

- Requires maintainers to exercise judgement per project

## Notes

This ADR should be revisited if:

- a project is explicitly positioned as production-ready,

- external users begin relying on a system,

- or security incidents occur that warrant stricter controls.
