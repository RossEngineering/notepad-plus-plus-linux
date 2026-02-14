# RC3 Blockers - v0.9.9-rc.3

Last updated: 2026-02-14

This list tracks prioritized blockers that must be resolved or explicitly deferred before cutting `v0.9.9-rc.3`.

## Prioritization scale

- `P0`: Release-blocking correctness, crash, data-loss, packaging, or security risk.
- `P1`: High user-impact issue that materially reduces release confidence.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| RC3-B01 | P0 | Consumer install UX | RC3 roadmap | Open | Consumer install/uninstall flow is consistent across target distros, with no unresolved desktop integration defects. |
| RC3-B02 | P0 | File-handler integration | RC3 roadmap | Open | File associations/default app/open-with behavior validates across Arch derivatives, Ubuntu LTS, and Fedora stable. |
| RC3-B03 | P1 | Release engineering dry-run | RC3 roadmap | Open | Reproducible artifact/checksum/signing flow completed and rollback path validated. |
| RC3-B04 | P1 | Full regression confidence | RC3 roadmap | Open | Required regression matrix passes (editor core, language workflows, skinning, extension lifecycle, crash recovery). |

## Intake process

1. New blocker candidates must include repro steps and expected vs actual behavior.
2. Each blocker must be linked to an issue/PR before status changes from `Open`.
3. Any defer/waive decision requires rationale and explicit target milestone.

## RC3 readiness rule

`v0.9.9-rc.3` requires:

1. Zero unresolved `P0` blockers.
2. All unresolved `P1` blockers documented with explicit defer rationale.
