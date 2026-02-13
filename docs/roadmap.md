# Roadmap

This timeline reflects project direction as of February 13, 2026.
Dates are target dates and may be adjusted as migration risk changes.

## Milestones

| Milestone | Target Date | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | February 13, 2026 | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | February 13, 2026 | Phase 8 completion (Linux-first docs, migration issue workflows, dashboard). | Done |
| M2: Beta 1 Readiness | April 15, 2026 | Phase 9 complete: automatic language detection + stable auto-highlighting behavior. | Planned |
| M3: Beta 1 Release (`v0.8.0-beta.1`) | April 30, 2026 | First beta cut. No additional alpha releases. | Planned |
| M4: Post-Beta UX Expansion | June 30, 2026 | Skinning and theming system delivery (Phase 10). | Planned |
| M5: Post-Beta Extension Platform | August 31, 2026 | Extension API + targeted VS Code language-asset compatibility path (Phase 11). | Planned |
| M6: 1.0 Release Candidate | October 15, 2026 | Feature freeze, compatibility validation, and blocker burn-down for RC. | Planned |

## Near-term priorities (next 6 weeks)

1. Implement automatic language detection pipeline (extension + shebang + content heuristics).
2. Lock lexer auto-selection behavior for Markdown, HTML, and core programming languages.
3. Define and enforce Beta 1 detection accuracy/false-positive thresholds.
4. Prepare Beta 1 release checklist and milestone release notes.

## Exit criteria for M2 (Beta 1 Readiness)

1. Linux CI green on all required workflows.
2. Automatic language detection works for targeted Markdown/HTML/programming corpus.
3. Lexer auto-selection false-positive rate remains below agreed threshold.
4. Beta 1 release candidate passes artifact/checksum validation.
