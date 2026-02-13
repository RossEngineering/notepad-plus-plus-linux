# RC2 Blockers - v0.9.6-rc.2

Last updated: 2026-02-13

This list tracks prioritized blockers that must be resolved or explicitly deferred before cutting `v0.9.6-rc.2`.

## Prioritization scale

- `P0`: Release-blocking correctness, crash, data-loss, or security risk.
- `P1`: High user-impact issue that materially reduces RC confidence.

## Current prioritized blockers

| ID | Priority | Area | Source | Status | Exit criteria |
| --- | --- | --- | --- | --- | --- |
| RC2-B01 | P0 | LSP UX wiring | RC2 roadmap + RC1 defer (`RC1-B04`) | Closed | Diagnostics, hover, and go-to-definition baseline are user-accessible and regression-tested through the Linux UI path. |
| RC2-B02 | P1 | Performance CI enforcement | RC2 roadmap | Closed | Startup/typing benchmark budgets are enforced in required CI lanes. |
| RC2-B03 | P1 | VS Code language asset compatibility breadth | RC2 roadmap | Closed | Regression corpus includes additional language assets plus multi-grammar edge-case coverage. |
| RC2-B04 | P1 | Distro packaging/install docs quality | RC2 roadmap | Closed | Arch derivatives, Ubuntu LTS, and Fedora install guidance is explicit, current, and linked from top-level docs. |

## Closed blocker evidence

1. `RC2-B02` (closed):
   - Added `scripts/check_startup_typing_budget.sh`.
   - Added required `performance-budget-check` job in `.github/workflows/linux-cmake.yml`.
   - Thresholds documented in `docs/performance-baseline.md`.
2. `RC2-B03` (closed):
   - Expanded fixtures with:
     - `rust-lang.rust-analyzer`
     - `vscode.markdown-baseline`
     - `example.multi-grammar`
   - Extended assertions in `tests/regression/vscode_language_asset_compatibility_test.cpp`.
3. `RC2-B04` (closed):
   - Added validated distro install guide: `docs/install-linux.md`.
   - Linked from `README.md` and `BUILD.md`.
4. `RC2-B01` (closed):
   - Added baseline LSP UX actions in the Language menu:
     - start/stop session
     - hover (baseline)
     - go-to-definition (baseline)
     - diagnostics (baseline)
   - Wired session lifecycle through `LinuxLspClientService` in `ui/qt/MainWindow.cpp`.
   - Added baseline helper module: `ui/qt/LspBaselineFeatures.cpp`.
   - Added regression coverage: `tests/regression/lsp_baseline_ux_regression_test.cpp`.

## RC2 readiness rule

`v0.9.6-rc.2` requires:

1. Zero unresolved `P0` blockers.
2. All unresolved `P1` blockers either closed or explicitly deferred with rationale and target.
