# Extension Authoring Guide (v1)

> **Linux-only fork notice:** This repository and its releases target Linux only. For the original Windows Notepad++ application, visit [notepad-plus-plus.org](https://notepad-plus-plus.org/).

Last updated: 2026-02-14

This guide focuses on the practical author workflow for Extension API v1:

1. language contributions (`language-pack`)
2. formatter contributions (`command-plugin` / `tool-integration`)
3. local marketplace index entries used by the in-app marketplace placeholder

Normative API and security constraints are defined in `docs/extension-api-v1.md`.

## 1) Choose extension type

- `language-pack`
  - No executable process.
  - Contributes grammar/config assets.
- `command-plugin`
  - Executable extension entrypoint.
  - Can contribute formatter handlers.
- `tool-integration`
  - Executable wrapper around external tools.
  - Can contribute formatter handlers.

## 2) Required layout

Each extension directory must contain:

1. `extension.json` (required)
2. `README.md` (required)
3. additional assets by type

Example executable extension layout:

```text
ross.sample.python-formatter/
  extension.json
  README.md
  bin/
    formatter-extension
```

## 3) Minimal manifest examples

### 3.1 Language pack

```json
{
  "schemaVersion": 1,
  "id": "ross.sample.markdown-pack",
  "name": "Sample Markdown Language Pack",
  "version": "0.1.0",
  "type": "language-pack",
  "apiVersion": "1.0",
  "description": "Example language assets package",
  "categories": ["language"]
}
```

### 3.2 Formatter-capable executable extension

```json
{
  "schemaVersion": 1,
  "id": "ross.sample.python-formatter",
  "name": "Sample Python Formatter",
  "version": "0.1.0",
  "type": "command-plugin",
  "apiVersion": "1.0",
  "entrypoint": "bin/formatter-extension",
  "permissions": [
    "process.spawn",
    "workspace.read",
    "workspace.write"
  ],
  "formatters": [
    {
      "languages": ["python"],
      "args": ["--stdin-filepath", "${filePath}", "--tab-width", "${tabWidth}"]
    }
  ]
}
```

## 4) Formatter runtime contract

When `Format Document` selects your formatter:

1. Current document is provided on `stdin` (UTF-8).
2. Arguments are resolved from `formatters[].args` placeholders.
3. Formatter must return full formatted document on `stdout`.
4. Exit code must be `0` for success.
5. Invalid UTF-8 or non-zero exit keeps the original document unchanged.

Permissions:

- executable formatter invocation is permission-gated (`process.spawn`).
- denied permission means formatter is skipped.

## 5) Install and test locally

Use the app UI:

1. `Tools -> Install Extension from Folder...`
2. choose the extension root containing `extension.json`

Then validate:

1. `Tools -> Manage Extensions...` shows installed version and startup/resource details.
2. `Edit -> Format Document` triggers your formatter for matching languages.

## 6) Local marketplace index (placeholder)

The in-app marketplace currently reads a **local index file** (no remote protocol in v1).

Search order:

1. `${XDG_CONFIG_HOME}/notepad-plus-plus-linux/extensions-marketplace-index.json`
2. `<appdir>/../share/notepad-plus-plus-linux/extensions/index.json`
3. `/usr/local/share/notepad-plus-plus-linux/extensions/index.json`
4. `/usr/share/notepad-plus-plus-linux/extensions/index.json`

Index format:

```json
{
  "schemaVersion": 1,
  "extensions": [
    {
      "id": "ross.sample.python-formatter",
      "name": "Sample Python Formatter",
      "version": "0.1.1",
      "description": "Example formatter update",
      "homepage": "https://github.com/example/sample-formatter",
      "sourceDirectory": "/absolute/or/relative/path/to/extension/folder"
    }
  ]
}
```

Notes:

- `sourceDirectory` may be absolute or relative to the index file directory.
- if `version` is newer than installed, the UI surfaces an update.
- update/install uses one-click `InstallFromDirectory` flow.

## 7) Troubleshooting

- Safe mode:
  - launch with `--safe-mode` (or `--safe-mode-no-extensions`) to disable all extensions.
  - use `Tools -> Relaunch in Normal Mode` to re-enable extension execution.
- Permission prompts:
  - denied permissions can be reset via `Tools -> Manage Extensions...` -> `Reset Permissions`.
- Manifest errors:
  - validate against `docs/schemas/extension-manifest-v1.schema.json`.
