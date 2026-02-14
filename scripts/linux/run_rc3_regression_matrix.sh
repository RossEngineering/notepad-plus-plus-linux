#!/usr/bin/env bash
set -euo pipefail

required_test_regex="language_detection_regression_test|skin_accessibility_regression_test|lexer_style_config_regression_test|lsp_baseline_ux_regression_test|lexilla_smoke_test|syntax_highlighting_smoke_test|scintilla_document_smoke_test|core_text_undo_redo_test|encoding_large_file_regression_test|platform_linux_services_smoke_test|extension_lifecycle_smoke_test|crash_recovery_persistence_regression_test|lsp_client_foundation_smoke_test|vscode_language_asset_compatibility_test"

run_preset() {
  local preset="$1"
  echo "==> Configure (${preset})"
  cmake --preset "${preset}"

  echo "==> Build (${preset})"
  cmake --build --preset "${preset}" -- -j2

  echo "==> Test (${preset})"
  ctest --test-dir "build/${preset}" --output-on-failure -R "${required_test_regex}"
}

run_preset debug
run_preset release

echo "RC3 regression matrix passed (debug + release)."
