# RossEngineering ADR Alignment Check (2026-02-13)

Reference set imported from:
`docs/adr/rossengineering-bafc82c7/`

## Overall assessment

- Status: **Partially aligned**
- Summary: Technical architecture/migration work is strong, but organisation-level governance items from RossEngineering ADRs are only partially implemented in this repo.

## Per-ADR alignment

1. `ADR-003 — Security Posture & Disclosure Policy`
   - Alignment: **partial**
   - Evidence present:
     - Security posture is implicitly scoped by migration docs and platform boundaries.
   - Gaps:
     - No repository `SECURITY.md`.
     - No explicit vulnerability disclosure path documented in this repo.

2. `ADR-004 — Versioning & Backward Compatibility Strategy`
   - Alignment: **partial**
   - Evidence present:
     - Compatibility intent documented in `docs/compatibility-target.md`.
     - Plugin compatibility constraints documented in `docs/plugin-strategy.md`.
   - Gaps:
     - No formal repository versioning policy doc for Linux releases.
     - Deprecation/compatibility process not explicitly captured as policy.

3. `ADR-005 — Repository Admission & Lifecycle Policy`
   - Alignment: **partial**
   - Evidence present:
     - Structured README and active roadmap (`TODO.md`).
     - CI workflows exist for Windows/Linux and guard checks.
   - Gaps:
     - `docs/decisions.md` (explicit decision log format requested by ADR-005) is missing.
     - Lifecycle states (promoted/incubator/archive semantics) are not explicitly declared in this repo.

4. `ADR-006 — Incubator Policy & Promotion Criteria`
   - Alignment: **partial**
   - Evidence present:
     - Current repo is clearly migration-stage and has phased plan.
   - Gaps:
     - Incubator status is not explicitly declared in README/docs.
     - Promotion criteria are not mapped to concrete exit gates in this repo.

5. `ADR-006a Repository Promotion Checklist`
   - Alignment: **mixed**
   - Satisfied areas:
     - Scope and roadmap clarity (`README.md`, `TODO.md`).
     - CI exists and is running.
     - Architectural structure and boundaries documented.
   - Missing/weak areas:
     - Required explicit decision log file `docs/decisions.md`.
     - Security reporting path file/policy.
     - Automated tests are present but currently smoke-level for core migration paths; broader coverage is still pending in Phase 7.

6. `ADR-008 — Testing & Quality Assurance Expectations`
   - Alignment: **partial**
   - Evidence present:
     - Automated tests exist and run in Linux CI (`ctest` smoke suite).
   - Gaps:
     - Current test scope is not yet deep enough to satisfy full behavioural confidence (already planned in Phase 7).

## Recommended follow-up actions

1. Add `SECURITY.md` with private disclosure channel and scope statement.
2. Add `docs/decisions.md` as repo-level decision index linking ADR records.
3. Add an explicit incubator-status note in `README.md` until promotion gates are met.
4. Add a short `docs/versioning-policy.md` for release numbering, compatibility, and deprecation expectations.
5. Convert key Phase 7 testing items into concrete CI gates (core text ops, encoding regressions, lexer/theme smoke suites).
