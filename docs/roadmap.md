# Roadmap

This timeline reflects project direction as of February 13, 2026.
Dates are target dates and may be adjusted as migration risk changes.

## Milestones

| Milestone | Target Date | Scope | Status |
| --- | --- | --- | --- |
| M0: Foundation Complete | February 13, 2026 | Phases 0-7 complete (architecture boundaries, Linux shell, packaging, quality baseline). | Done |
| M1: Developer UX Baseline | February 13, 2026 | Phase 8 completion (Linux-first docs, migration issue workflows, dashboard). | Done |
| M2: Beta 1 Readiness | April 15, 2026 | Phase 9 complete: automatic language detection + stable auto-highlighting behavior. | Done |
| M3: Beta 1 Release (`v0.8.0-beta.1`) | April 30, 2026 | First beta cut. No additional alpha releases. | Done |
| M4: Beta 2 Workstream A | June 30, 2026 | Phase 10 skinning/theming delivery. | Planned |
| M5: Beta 2 Workstreams B+C and Release (`v0.9.0-beta.2`) | August 31, 2026 | Phases 11-12 completion (extensions + hardening/language-intelligence) and Beta 2 cut. | Planned |
| M6: 1.0 Release Candidate | October 15, 2026 | Feature freeze, compatibility validation, and blocker burn-down for RC. | Planned |

## Near-term priorities (next 6 weeks)

1. Finalize Phase 10 skin/theme format and first-party light/dark/high-contrast packs.
2. Implement runtime skin switching with persisted user preference.
3. Define Phase 11 extension API boundaries and publish VS Code compatibility ADR.
4. Start Phase 12 LSP/autocomplete foundation and crash-recovery hardening work.

## Exit criteria for M2 (Beta 1 Readiness)

1. Linux CI green on all required workflows.
2. Automatic language detection works for targeted Markdown/HTML/programming corpus.
3. Lexer auto-selection false-positive rate remains below agreed threshold.
4. Beta 1 release candidate passes artifact/checksum validation.

## Beta 1 state (2026-02-13)

1. Scope frozen to Phase 9 deliverables.
2. Linux CI gates (including sanitizer checks) are green.
3. Release artifacts/checksums validated.
4. `v0.8.0-beta.1` tag and GitHub Release are published.

## Beta 2 definition (2026-02-13)

1. Beta 2 scope is Phases 10-12.
2. Phase 10: UI skinning support.
3. Phase 11: extension platform and VS Code language-asset compatibility path.
4. Phase 12: hardening and language-intelligence improvements.
5. Planned release target for this scope: `v0.9.0-beta.2`.

## Phase 12 status (2026-02-13)

1. LSP client foundation is implemented.
2. Syntax-aware autocomplete assists are implemented (HTML/XML tag auto-close and paired delimiters).
3. Crash-recovery journal and startup recovery prompt are implemented.
4. Extension startup performance guardrails are implemented.
5. Distro validation expansion plan is documented.
