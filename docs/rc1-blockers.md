# RC1 Blockers - v0.9.3-rc.1

Last updated: 2026-02-13

This list tracks prioritized blockers that must be resolved or explicitly waived before cutting `v0.9.3-rc.1`.

## Prioritization scale

- `P0`: Release-blocking correctness, crash, data-loss, or security risk.
- `P1`: High user-impact issue that materially reduces RC confidence.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| RC1-B01 | P0 | Extension permissions UX | Beta feedback + RC1 plan | Closed | Deny/allow flows are explicit, reversible, and tested (filesystem/network/process prompts). |
| RC1-B02 | P0 | Syntax color consistency | Beta feedback + RC1 plan | Closed | Color-coded syntax output is consistent across targeted lexers/themes and validated by regression checks. |
| RC1-B03 | P0 | Cross-distro baseline | Beta feedback + RC1 plan | Closed | Arch derivatives, Ubuntu LTS, and Fedora stable each have captured build/test/launch evidence with no unresolved P0 defects. |
| RC1-B04 | P1 | LSP baseline confidence | Known limitation in Beta 2 notes | Deferred (RC2) | LSP baseline paths remain stable (no regressions) and RC1 scope limits are documented. |
| RC1-B05 | P1 | Crash/correctness burn-down | RC1 stabilization goal | Closed | All open beta-era P0/P1 crash/correctness issues are fixed or have approved defer rationale. |

## Intake process

1. New blocker candidates must include repro steps and expected vs actual behavior.
2. Each blocker must be linked to an issue/PR before status changes from `Open`.
3. Any defer/waive decision requires rationale and explicit target milestone.

## Closed blocker evidence

1. `RC1-B01` (closed):
   - User-denied permission requests now fail closed with explicit `kPermissionDenied` host errors.
   - Install rollback removes copied extension files when interactive permission is denied.
   - Permission decisions are reversible via `Reset Permissions` in extension management UI.
   - Regression coverage updated in `tests/platform/extension_lifecycle_smoke_test.cpp`.
2. `RC1-B02` (closed):
   - Introduced shared lexer style mapping in `ui/qt/LexerStyleConfig.h` and applied it in `ui/qt/MainWindow.cpp`.
   - Added YAML and SQL custom syntax color-role mapping and language menu parity.
   - Extended syntax smoke coverage in `tests/smoke/syntax_highlighting_smoke_test.cpp` (`xml`, `markdown`, `yaml`, `sql`).
   - Added regression gate `tests/regression/lexer_style_config_regression_test.cpp`.
3. `RC1-B03` (closed):
   - Completed RC1 distro matrix baseline on commit `bc1ea6cba`.
   - Arch derivatives host lane: debug/release build + `ctest` (`12/12` each) + install + offscreen launch smoke.
   - Ubuntu lane (`ubuntu:24.04` Docker): debug/release build + `ctest` (`12/12` each) + install + offscreen launch smoke.
   - Fedora lane (`fedora:latest` Docker): debug/release build + `ctest` (`12/12` each) + install + offscreen launch smoke.
   - Evidence captured in `docs/distro-validation-report-rc1-2026-02-13.md`.
4. `RC1-B05` (closed):
   - Beta-era P0/P1 correctness and stability fixes from RC1 have landed (`RC1-B01` to `RC1-B03`).
   - Current candidate validation:
     - `ctest --preset debug --output-on-failure` (`12/12`)
     - `ctest --preset release --output-on-failure` (`12/12`)
     - ASan/UBSan subset suite (`10/10`) with local `detect_leaks=0` due sandbox ptrace limitation.
     - `./scripts/check_win32_boundaries.sh HEAD~1` pass.
   - Release artifact build/checksum validation completed for `v0.9.3-rc.1`.

## Deferred blocker rationale

1. `RC1-B04` (deferred to RC2):
   - Baseline LSP path remains stable:
     - `tests/platform/lsp_client_foundation_smoke_test.cpp` passes in debug/release.
     - Included in local ASan/UBSan subset lane for RC1 candidate.
   - RC1 scope intentionally limits LSP to baseline confidence and regression stability.
   - Deeper user-facing LSP wiring (diagnostics/hover/go-to-definition UX completeness) remains explicitly planned for RC2 in `TODO.md` and `docs/roadmap.md`.

## RC1 readiness rule

`v0.9.3-rc.1` requires:

1. Zero unresolved `P0` blockers.
2. All unresolved `P1` blockers documented with explicit defer rationale.
