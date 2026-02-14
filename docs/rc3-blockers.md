# RC3 Blockers - v0.9.9-rc.3

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14 (post-RC2 exit review)

This list tracks prioritized blockers that must be resolved or explicitly deferred before cutting `v0.9.9-rc.3`.

## Prioritization scale

- `P0`: Release-blocking correctness, crash, data-loss, packaging, or security risk.
- `P1`: High user-impact issue that materially reduces release confidence.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| RC3-B01 | P0 | Consumer install UX | RC2 distro validation + RC3 roadmap | Closed | Consumer install/uninstall flow is consistent across target distros, with no unresolved desktop integration defects. |
| RC3-B02 | P0 | File-handler integration | RC2 distro validation + RC3 roadmap | Closed | File associations/default app/open-with behavior validates across Arch derivatives, Ubuntu LTS, and Fedora stable. |
| RC3-B03 | P1 | Release engineering dry-run | RC2 release notes follow-up + RC3 roadmap | Closed | Reproducible artifact/checksum/signing flow completed and rollback path validated. |
| RC3-B04 | P1 | Full regression confidence | RC2 release notes follow-up + RC3 roadmap | Closed | Required regression matrix passes (editor core, language workflows, skinning, extension lifecycle, crash recovery). |

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
3. Validated full integration matrix across all required distros (2026-02-14):
   - `./scripts/linux/run-desktop-integration-matrix.sh arch-derivative`
   - `./scripts/linux/run-desktop-integration-matrix.sh ubuntu-lts`
   - `./scripts/linux/run-desktop-integration-matrix.sh fedora-stable`
4. Recorded RC3 distro integration evidence:
   - `docs/distro-validation-report-rc3-2026-02-14.md`
5. Added explicit RC3 regression matrix gate and crash-recovery persistence regression coverage:
   - `scripts/linux/run_rc3_regression_matrix.sh`
   - `.github/workflows/linux-cmake.yml` (`rc3-regression-matrix`)
   - `tests/regression/crash_recovery_persistence_regression_test.cpp`
   - `docs/rc3-regression-report-2026-02-14.md`
6. Added release dry-run harness and completed RC3 release-engineering validation:
   - `scripts/release/run_release_dry_run.sh`
   - `docs/release-dry-run-report-rc3-2026-02-14.md`

## Intake process

1. New blocker candidates must include repro steps and expected vs actual behavior.
2. Each blocker must be linked to an issue/PR before status changes from `Open`.
3. Any defer/waive decision requires rationale and explicit target milestone.

## RC3 readiness rule

`v0.9.9-rc.3` requires:

1. Zero unresolved `P0` blockers.
2. All unresolved `P1` blockers documented with explicit defer rationale.
