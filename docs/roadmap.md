# Roadmap

This timeline reflects project direction as of February 13, 2026.
Dates are target dates and may be adjusted as migration risk changes.

## Milestones

| Milestone | Target Date | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | February 13, 2026 | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | February 20, 2026 | Phase 8 completion, Linux-first docs, migration issue workflows, release prep. | In Progress |
| M2: Linux Alpha Hardening | March 31, 2026 | Expand regression coverage, stabilize session/search behavior, improve crash diagnostics and startup path. | Planned |
| M3: Linux Beta Readiness | May 31, 2026 | Plugin API direction implementation, broader distro validation, packaging/signing polish. | Planned |
| M4: 1.0 Release Candidate | July 15, 2026 | Feature freeze, compatibility validation, release candidate cycle and blocker burn-down. | Planned |

## Near-term priorities (next 4 weeks)

1. Complete Phase 8 documentation and contributor UX tasks.
2. Cut next tagged alpha release with updated release notes and artifacts.
3. Increase automated coverage around file handling, session restore, and UI shell behavior.
4. Define acceptance criteria for Linux beta entry.

## Exit criteria for M2

1. Linux CI green on all required workflows.
2. No open critical regressions in core editing path.
3. Reproducible package build validated from clean Manjaro environment.
4. Updated migration dashboard in `README.md` reflects current phase status.
