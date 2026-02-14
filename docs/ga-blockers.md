# GA Blockers - v1.0.0

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14
Owner: Maintainers
Status: No open P0 blockers (maintain active monitoring)

This list tracks prioritized blockers that must be resolved or explicitly
deferred before cutting `v1.0.0`.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| GA-B01 | P0 | CI continuity gate | `docs/releases/v1.0.0-checklist.md` | Closed | 7-day continuity gate vetoed for `v1.0.0` under maintainer-approved exception; compensating controls and rationale recorded in `docs/ga-gate-exception-2026-02-14.md`. |
| GA-B02 | P0 | Release notes and migration docs | `docs/v1.0.0-go-no-go-review-2026-02-14.md` | Closed | `docs/releases/v1.0.0.md` finalized on 2026-02-14 with migration guidance and accepted known limitations. |
| GA-B03 | P0 | Incubator lifecycle decision | `docs/incubator-promotion-validation-2026-02-14.md` | Closed | Promotion approved and recorded at `v0.10.0-rc.4`; repository status/governance text synchronized. |

## Intake and closure rules

1. New GA blocker candidates must include reproducible evidence and a clear
   user/release impact statement.
2. Each blocker must be linked to a PR/issue before status changes from `Open`.
3. `P0` blockers must be closed before tagging `v1.0.0`.
4. Any deferred `P1` blocker must include explicit rationale and an owner.
5. Triage classification and escalation should follow
   `docs/ga-triage-playbook.md`, with environment coverage tracked in
   `docs/tester-coverage-snapshot-2026-02.md`.

## GA readiness rule

`v1.0.0` may be tagged only when:

1. Zero unresolved `P0` blockers remain in this file.
2. The `v1.0.0` release checklist is fully complete.
3. The go/no-go review is updated with final decision and evidence links.
