# RC3 Regression Report - 2026-02-14

This report records the RC3 full-regression matrix run that validates:

1. editor core behavior
2. language workflows
3. skinning behavior
4. extension lifecycle behavior
5. crash-recovery persistence path

## Required matrix command

```bash
./scripts/linux/run_rc3_regression_matrix.sh
```

## Result

- Status: pass
- Presets executed: `debug`, `release`
- Tests executed per preset: `14/14`
- Total failures: `0`

## Required test set

1. `language_detection_regression_test`
2. `skin_accessibility_regression_test`
3. `lexer_style_config_regression_test`
4. `lsp_baseline_ux_regression_test`
5. `lexilla_smoke_test`
6. `syntax_highlighting_smoke_test`
7. `scintilla_document_smoke_test`
8. `core_text_undo_redo_test`
9. `encoding_large_file_regression_test`
10. `platform_linux_services_smoke_test`
11. `extension_lifecycle_smoke_test`
12. `crash_recovery_persistence_regression_test`
13. `lsp_client_foundation_smoke_test`
14. `vscode_language_asset_compatibility_test`

## Notes

- The matrix is now codified in:
  - `scripts/linux/run_rc3_regression_matrix.sh`
  - `.github/workflows/linux-cmake.yml` (`rc3-regression-matrix` job)
- Crash-recovery persistence coverage is explicitly validated by:
  - `tests/regression/crash_recovery_persistence_regression_test.cpp`
