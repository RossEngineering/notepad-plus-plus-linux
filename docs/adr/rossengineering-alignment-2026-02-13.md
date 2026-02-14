# RossEngineering ADR Alignment Check (2026-02-13)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Reference set imported from:
`docs/adr/rossengineering-bafc82c7/`

## Overall assessment

- Status: **Mostly aligned**
- Summary: Core governance documentation gaps were addressed (security, decisions log, incubator declaration, versioning policy). Primary remaining gap is test depth.

## Per-ADR alignment

1. `ADR-003 — Security Posture & Disclosure Policy`
   - Alignment: **aligned**
   - Evidence present:
     - `SECURITY.md` defines private reporting path and disclosure expectations.

2. `ADR-004 — Versioning & Backward Compatibility Strategy`
   - Alignment: **aligned**
   - Evidence present:
     - Compatibility intent documented in `docs/compatibility-target.md`.
     - Plugin compatibility constraints documented in `docs/plugin-strategy.md`.
     - Repository versioning/deprecation guidance in `docs/versioning-policy.md`.

3. `ADR-005 — Repository Admission & Lifecycle Policy`
   - Alignment: **mostly aligned**
   - Evidence present:
     - Structured README and active roadmap (`TODO.md`).
     - CI workflows exist for Windows/Linux and guard checks.
     - `docs/decisions.md` now provides repository decision index.
     - README now explicitly declares incubator status.

4. `ADR-006 — Incubator Policy & Promotion Criteria`
   - Alignment: **aligned (incubator stage)**
   - Evidence present:
     - Current repo is explicitly marked incubator in `README.md`.
     - Migration and promotion progress is tracked in `TODO.md`.

5. `ADR-006a Repository Promotion Checklist`
   - Alignment: **mixed**
   - Satisfied areas:
     - Scope and roadmap clarity (`README.md`, `TODO.md`).
     - CI exists and is running.
     - Architectural structure and boundaries documented.
     - Decision index added (`docs/decisions.md`).
     - Security policy added (`SECURITY.md`).
   - Missing/weak areas:
     - Automated tests are present but currently smoke-level for core migration paths; broader coverage is still pending in Phase 7.

6. `ADR-008 — Testing & Quality Assurance Expectations`
   - Alignment: **partial**
   - Evidence present:
     - Automated tests exist and run in Linux CI (`ctest` smoke suite).
   - Gaps:
     - Current test scope is not yet deep enough to satisfy full behavioural confidence (already planned in Phase 7).

## Recommended follow-up actions

1. Convert key Phase 7 testing items into concrete CI gates (core text ops, encoding regressions, lexer/theme smoke suites).
2. Re-run promotion checklist after Phase 7 quality items are complete.
