# Roadmap

This timeline reflects project direction as of February 13, 2026.
Dates are target dates and may be adjusted as migration risk changes.

## Milestones

| Milestone | Target Date | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | February 13, 2026 | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | February 13, 2026 | Phase 8 completion (Linux-first docs, migration issue workflows, dashboard). | Done |
| M2: Smart Language + Skinning Alpha | April 30, 2026 | Automatic language detection + highlighting and first skinning system (Phases 9-10). | Planned |
| M3: Extension Platform Alpha | June 30, 2026 | Linux extension API and targeted VS Code language-asset compatibility path (Phase 11). | Planned |
| M4: Linux Beta Hardening | August 31, 2026 | LSP/crash-recovery/performance hardening and broader distro validation (Phase 12). | Planned |
| M5: 1.0 Release Candidate | October 15, 2026 | Feature freeze, compatibility validation, and blocker burn-down for RC. | Planned |

## Near-term priorities (next 6 weeks)

1. Implement automatic language detection pipeline (extension + shebang + content heuristics).
2. Lock lexer auto-selection behavior for Markdown, HTML, and core programming languages.
3. Draft and implement skin schema with first-party light/dark/high-contrast skins.
4. Publish VS Code compatibility ADR and confirm targeted compatibility boundary.

## Exit criteria for M2 (Smart Language + Skinning Alpha)

1. Linux CI green on all required workflows.
2. Automatic language detection works for targeted Markdown/HTML/programming corpus.
3. Lexer auto-selection false-positive rate remains below agreed threshold.
4. Skinning system supports complete shell coverage and accessibility checks.
