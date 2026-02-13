# Extension Performance Guardrails

Last updated: 2026-02-13

## Purpose

Detect and surface extension startup impact regressions early in local runs and CI-equivalent workflows.

## Startup guardrails

At application startup, extension initialization and discovery are measured:

1. `extension_init_ms`
2. `extension_discover_ms`
3. `extension_startup_total_ms`
4. `extension_per_extension_ms`

If thresholds are exceeded, the app:

1. Shows a startup warning in the status bar.
2. Writes a diagnostic log under the app diagnostics log category.

## Default thresholds

- `extensionStartupBudgetMs`: `1200`
- `extensionPerExtensionBudgetMs`: `250`

These are configurable via `editor-settings.json`.

## Settings keys

- `extensionGuardrailsEnabled` (default: `true`)
- `extensionStartupBudgetMs` (default: `1200`)
- `extensionPerExtensionBudgetMs` (default: `250`)

