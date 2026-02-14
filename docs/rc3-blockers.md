# RC3 Blockers - v0.9.9-rc.3

Last updated: 2026-02-14 (post-RC2 exit review)

This list tracks prioritized blockers that must be resolved or explicitly deferred before cutting `v0.9.9-rc.3`.

## Prioritization scale

- `P0`: Release-blocking correctness, crash, data-loss, packaging, or security risk.
- `P1`: High user-impact issue that materially reduces release confidence.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| RC3-B01 | P0 | Consumer install UX | RC2 distro validation + RC3 roadmap | In Progress | Consumer install/uninstall flow is consistent across target distros, with no unresolved desktop integration defects. |
| RC3-B02 | P0 | File-handler integration | RC2 distro validation + RC3 roadmap | In Progress | File associations/default app/open-with behavior validates across Arch derivatives, Ubuntu LTS, and Fedora stable. |
| RC3-B03 | P1 | Release engineering dry-run | RC2 release notes follow-up + RC3 roadmap | Open | Reproducible artifact/checksum/signing flow completed and rollback path validated. |
| RC3-B04 | P1 | Full regression confidence | RC2 release notes follow-up + RC3 roadmap | Open | Required regression matrix passes (editor core, language workflows, skinning, extension lifecycle, crash recovery). |

## RC2 exit signals used for RC3 intake

1. `docs/rc2-blockers.md`: all RC2 blockers closed, with remaining scope explicitly deferring GA-readiness concerns to RC3.
2. `docs/distro-validation-report-rc2-2026-02-14.md`: distro coverage called out as pending final-tag confidence for Ubuntu/Fedora lanes.
3. `docs/releases/v0.9.6-rc.2.md`: known limitations identify remaining release-readiness and advanced validation work.
4. `TODO.md` RC3 program: freeze, install UX, file-handler validation, regression sweep, and release dry-run are explicit release gates.

## Progress since RC2 cut

1. Added user-local installer/uninstaller flow and consumer install guide:
   - `scripts/linux/install-local.sh`
   - `scripts/linux/uninstall-local.sh`
   - `docs/install-consumer-linux.md`
2. Added distro-matrix desktop/file-handler validation lane:
   - `.github/workflows/linux-desktop-integration.yml`
   - `scripts/linux/run-desktop-integration-matrix.sh`
   - `scripts/linux/validate-desktop-integration.sh`
   - `scripts/linux/validate-file-handler-defaults.sh`

## Intake process

1. New blocker candidates must include repro steps and expected vs actual behavior.
2. Each blocker must be linked to an issue/PR before status changes from `Open`.
3. Any defer/waive decision requires rationale and explicit target milestone.

## RC3 readiness rule

`v0.9.9-rc.3` requires:

1. Zero unresolved `P0` blockers.
2. All unresolved `P1` blockers documented with explicit defer rationale.
